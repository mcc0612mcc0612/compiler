#include "GVN.h"

#include "BasicBlock.h"
#include "Constant.h"
#include "DeadCode.h"
#include "FuncInfo.h"
#include "Function.h"
#include "Instruction.h"
#include "logging.hpp"

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <fstream>
#include <memory>
#include <sstream>
#include <tuple>
#include <utility>
#include <vector>

using namespace GVNExpression;
using std::string_literals::operator""s;
using std::shared_ptr;

static auto get_const_int_value = [](Value *v) { return dynamic_cast<ConstantInt *>(v)->get_value(); };
static auto get_const_fp_value = [](Value *v) { return dynamic_cast<ConstantFP *>(v)->get_value(); };
// Constant Propagation helper, folders are done for you
Constant *ConstFolder::compute(Instruction *instr, Constant *value1, Constant *value2) {
    auto op = instr->get_instr_type();
    switch (op) {
    case Instruction::add: return ConstantInt::get(get_const_int_value(value1) + get_const_int_value(value2), module_);
    case Instruction::sub: return ConstantInt::get(get_const_int_value(value1) - get_const_int_value(value2), module_);
    case Instruction::mul: return ConstantInt::get(get_const_int_value(value1) * get_const_int_value(value2), module_);
    case Instruction::sdiv: return ConstantInt::get(get_const_int_value(value1) / get_const_int_value(value2), module_);
    case Instruction::fadd: return ConstantFP::get(get_const_fp_value(value1) + get_const_fp_value(value2), module_);
    case Instruction::fsub: return ConstantFP::get(get_const_fp_value(value1) - get_const_fp_value(value2), module_);
    case Instruction::fmul: return ConstantFP::get(get_const_fp_value(value1) * get_const_fp_value(value2), module_);
    case Instruction::fdiv: return ConstantFP::get(get_const_fp_value(value1) / get_const_fp_value(value2), module_);

    case Instruction::cmp:
        switch (dynamic_cast<CmpInst *>(instr)->get_cmp_op()) {
        case CmpInst::EQ: return ConstantInt::get(get_const_int_value(value1) == get_const_int_value(value2), module_);
        case CmpInst::NE: return ConstantInt::get(get_const_int_value(value1) != get_const_int_value(value2), module_);
        case CmpInst::GT: return ConstantInt::get(get_const_int_value(value1) > get_const_int_value(value2), module_);
        case CmpInst::GE: return ConstantInt::get(get_const_int_value(value1) >= get_const_int_value(value2), module_);
        case CmpInst::LT: return ConstantInt::get(get_const_int_value(value1) < get_const_int_value(value2), module_);
        case CmpInst::LE: return ConstantInt::get(get_const_int_value(value1) <= get_const_int_value(value2), module_);
        }
    case Instruction::fcmp:
        switch (dynamic_cast<FCmpInst *>(instr)->get_cmp_op()) {
        case FCmpInst::EQ: return ConstantInt::get(get_const_fp_value(value1) == get_const_fp_value(value2), module_);
        case FCmpInst::NE: return ConstantInt::get(get_const_fp_value(value1) != get_const_fp_value(value2), module_);
        case FCmpInst::GT: return ConstantInt::get(get_const_fp_value(value1) > get_const_fp_value(value2), module_);
        case FCmpInst::GE: return ConstantInt::get(get_const_fp_value(value1) >= get_const_fp_value(value2), module_);
        case FCmpInst::LT: return ConstantInt::get(get_const_fp_value(value1) < get_const_fp_value(value2), module_);
        case FCmpInst::LE: return ConstantInt::get(get_const_fp_value(value1) <= get_const_fp_value(value2), module_);
        }
    default: return nullptr;
    }
}

Constant *ConstFolder::compute(Instruction *instr, Constant *value1) {
    auto op = instr->get_instr_type();
    switch (op) {
    case Instruction::sitofp: return ConstantFP::get((float)get_const_int_value(value1), module_);
    case Instruction::fptosi: return ConstantInt::get((int)get_const_fp_value(value1), module_);
    case Instruction::zext: return ConstantInt::get((int)get_const_int_value(value1), module_);
    default: return nullptr;
    }
}

namespace utils {
static std::string print_congruence_class(const CongruenceClass &cc) {
    std::stringstream ss;
    if (cc.index_ == 0) {
        ss << "top class\n";
        return ss.str();
    }
    ss << "\nindex: " << cc.index_ << "\nleader: " << cc.leader_->print()
       << "\nvalue phi: " << (cc.value_phi_ ? cc.value_phi_->print() : "nullptr"s)
       << "\nvalue expr: " << (cc.value_expr_ ? cc.value_expr_->print() : "nullptr"s) << "\nmembers: {";
    for (auto &member : cc.members_)
        ss << member->print() << "; ";
    ss << "}\n";
    return ss.str();
}

static std::string dump_cc_json(const CongruenceClass &cc) {
    std::string json;
    json += "[";
    for (auto member : cc.members_) {
        if (auto c = dynamic_cast<Constant *>(member))
            json += member->print() + ", ";
        else
            json += "\"%" + member->get_name() + "\", ";
    }
    json += "]";
    return json;
}

static std::string dump_partition_json(const GVN::partitions &p) {
    std::string json;
    json += "[";
    for (auto cc : p)
        json += dump_cc_json(*cc) + ", ";
    json += "]";
    return json;
}

static std::string dump_bb2partition(const std::map<BasicBlock *, GVN::partitions> &map) {
    std::string json;
    json += "{";
    for (auto [bb, p] : map)
        json += "\"" + bb->get_name() + "\": " + dump_partition_json(p) + ",";
    json += "}";
    return json;
}

// logging utility for you
static void print_partitions(const GVN::partitions &p) {
    if (p.empty()) {
        LOG_DEBUG << "empty partitions\n";
        return;
    }
    std::string log;
    for (auto &cc : p)
        log += print_congruence_class(*cc);
    LOG_DEBUG << log; // please don't use std::cout
}
} // namespace utils

GVN::partitions GVN::join(const partitions &P1, const partitions &P2) {
    // TODO: do intersection pair-wise
    partitions P;
    for(auto &c1 : P1 ){
        for(auto &c2 : P2){
            auto ck = intersect(c1,c2);
            if(ck) P.insert(ck);
        }
    }
    return P;
}

std::shared_ptr<CongruenceClass> GVN::intersect(std::shared_ptr<CongruenceClass> Ci,
                                                std::shared_ptr<CongruenceClass> Cj) {
    // TODO
    return {};
}

void GVN::detectEquivalences(){
    bool changed = true;
    //std::map<BasicBlock *, partitions> pin_, pout_;
    //using partitions = std::set<std::shared_ptr<CongruenceClass>>;
    int index = 1;
    for (auto &bb : func_->get_basic_blocks()){
        struct CongruenceClass Top = CongruenceClass(-1);//index=-1标记
        if(index == 1) {
            pin_[&bb] = {};
        }
        partitions top; 
        top.insert(shared_ptr<CongruenceClass>(&Top));
        pout_[&bb] = top;
        index++;
    }
    partitions pout;
    // initialize pout with top
    // iterate until converge
    while (changed){
        changed = false;
        // see the pseudo code in documentation
        for (auto &bb : func_->get_basic_blocks()) { // you might need to visit the blocks in depth-first order
            /*-----------------------------------处理块内指令----------------------------------*/
            int is_first = 1;//是块中第一个指令
            for(auto &instr : bb.get_instructions()){
                if(is_first == 1){
                    if(bb.get_pre_basic_blocks().size() == 1){
                        //PINs = POUTs3 
                        auto it = bb.get_pre_basic_blocks().begin();
                        pin_[&bb] = pout_[*it];
                    }
                    else if(bb.get_pre_basic_blocks().size() == 2){
                        auto P1 = *(bb.get_pre_basic_blocks().begin());
                        auto P2 = *(bb.get_pre_basic_blocks().end());
                        pin_[&bb] = join(pout_[P1],pout_[P2]);
                    }
                    is_first = 0;
                }
                else if(is_first == 0){
                    pin_[&bb] = pout_[&bb];
                }
                if(instr.is_phi());
                else pout = transferFunction(&instr,nullptr,pin_[&bb]);
                if(pout == pout_[&bb]);
                else{
                    changed = true;
                    pout_[&bb] = pout;
                }
            }
            /*-----------------------------处理后继节点的phi指令---------------------------------*/
            for(auto &succ_bb : bb.get_succ_basic_blocks()){
                for(auto &instr : succ_bb->get_instructions()){//后继块的phi前移
                    if(instr.is_phi()){
                        if((instr.get_operand(1)) == (dynamic_cast<Value*> (&bb)))//比较地址？？
                            pout = transferFunction(&instr,instr.get_operand(0),pin_[&bb]);
                        else if((instr.get_operand(3)) == (dynamic_cast<Value*> (&bb)))//比较地址？？
                            pout = transferFunction(&instr,instr.get_operand(2),pin_[&bb]);
                        else{
                            printf("ERROR!后继节点参数未找到!\n");
                            return ;
                        }
                    }
                    if(pout == pout_[&bb]);
                    else{
                        changed = true;
                        pout_[&bb] = pout;
                    }
                }
            }
            // get PIN of bb by predecessor(s)
            // iterate through all instructions in the block
            // and the phi instruction in all the successors
            // check changes in pout
        }
    }
}
//寻找target是否在对应的集合,若在则删除它
bool GVN::extract(partitions pin, Value* target){
    bool is_find = false;
    for(auto it = pin.begin();it != pin.end(); it++){
        auto valueset = (*it) -> members_;
        auto it_ = valueset.begin();
        for(;it_ != valueset.end();it_++){
            if(*it_ == target) {
                valueset.erase(it_);is_find = true;
                if((*it)->leader_ == target) 
                    *((*it)->leader_) = *(*(valueset.begin()));//如果leader == target，删除leader
                return is_find;
            }
        }
    }
    return is_find;    
}

shared_ptr<Expression> GVN::findexpr(partitions pin, Value* target){
    for(auto it = pin.begin();it != pin.end();it++){
        auto valueset = (*it)->members_;//在members寻找target
        for(auto it_ = valueset.begin();it_ != valueset.end();it_++){
            if(*it_ == target) return (*it) -> value_expr_;
        }
    }
    return nullptr;
}

//Expression-value_expr_版:查找并且插入
bool GVN::findexp_insert(partitions pin, shared_ptr<Expression> target, Value* x){
    for(auto it = pin.begin();it != pin.end();it++){
        if (target == (*it)->value_expr_){//在value_expr_寻找target
            //Ci = Ci ∪ {x, ve}
            (*it) -> members_.insert(x);
            return true;
        }
    }
    return false;
}

//Expression-value_phi_版:查找并且插入
bool GVN::findvpf_insert(partitions pin, shared_ptr<PhiExpression> target, Value* x){
    if(!target) return false;
    for(auto it = pin.begin();it != pin.end();it++){
        if (*target == *((*it)->value_phi_)){//在value_phi_寻找target
            (*it)->members_.insert(x);
            return true;
        }
    }
    return false;
}


//将instr左值转化为phi+phi
shared_ptr<Expression> GVN::valueExpr(Instruction *instr,Value *c, partitions pin) {
    // TODO   
    shared_ptr<Expression> expr;
    //算术表达式
    if(instr == nullptr){
        auto expr = findexpr(pin,c);
        if(expr){
            ;
        }
        else{
            Constant* constant_op = dynamic_cast<Constant *>(c);
            if(constant_op)//常量 or 变量？
                expr = std::dynamic_pointer_cast<ConstantExpression>(expr)->create(constant_op);
            else{
                expr = std::dynamic_pointer_cast<VariableExpression>(expr)->create(c);
                printf("ERROR: undefined variable!!!");
                return 0;
            }
        }
    }
    if(instr->get_instr_type() == Instruction::add || instr->get_instr_type() == Instruction::sub || instr->get_instr_type() == Instruction::mul || 
    instr->get_instr_type() == Instruction::sdiv || instr->get_instr_type() == Instruction::fadd || instr->get_instr_type() == Instruction::fsub || 
    instr->get_instr_type() == Instruction::fmul || instr->get_instr_type() == Instruction::fdiv ){
        Value* op0 = instr->get_operand(0);
        auto exp0 = valueExpr(nullptr,op0,pin);//寻找op等价类
        Value* op1 = instr->get_operand(1);
        auto exp1 = valueExpr(nullptr,op1,pin);
        expr = std::dynamic_pointer_cast<BinaryExpression>(expr)->create(instr->get_instr_type(),exp0,exp1);
    }
    else if(instr->get_instr_type() == Instruction::getelementptr){
        for(int i = 0;i < instr->get_num_ops();i++){
            auto ve = instr->get_operand(i);
            auto exp = valueExpr(nullptr,ve,pin);
            std::dynamic_pointer_cast<GepExpression> (expr)->idxs.push_back(exp);
            //std::make_shared
        }
    }
    else if(instr->get_instr_type() == Instruction::phi){
        expr = valueExpr(nullptr,c,pin);
    }
    else if(instr->get_instr_type() == Instruction::cmp || instr->get_instr_type() == Instruction::fcmp){
        Value* op0 = instr->get_operand(0);
        auto exp0 = valueExpr(nullptr,op0,pin);//寻找op等价类
        Value* op1 = instr->get_operand(1);
        auto exp1 = valueExpr(nullptr,op1,pin);
        expr = std::dynamic_pointer_cast<CmpExpression> (expr)->create(dynamic_cast<CmpInst*>(instr)->get_cmp_op(),exp0,exp1);
    }
    else if(instr->get_instr_type() == Instruction::zext || instr->get_instr_type() == Instruction::fptosi || instr->get_instr_type() == Instruction::sitofp){
        Value* op = instr->get_operand(0);
        auto exp = valueExpr(nullptr,op,pin);
        expr = std::dynamic_pointer_cast<ZextExpression> (expr)->create(exp);
    }
    return std::dynamic_pointer_cast<Expression>(expr);
}

// instruction of the form `x = e`, mostly x is just e (SSA), but for copy stmt x is a phi instruction in the
// successor. Phi values (not copy stmt) should be handled in detectEquiv
/// \param bb basic block in which the transfer function is called
GVN::partitions GVN::transferFunction(Instruction *x, Value *e, partitions pin) {
    partitions pout = clone(pin);
    /*auto cc = createCongruenceClass(next_value_number_++);
    cc->leader_ = x;
    cc->members_={x};
    cc->value_expr_ = valueExpr(e);*/
    auto it = pout.begin();
    bool is_find = extract(pin,x);
    shared_ptr<Expression> ve;
    if(e){//是copy stmt; x3=phi(x1,x2)
        ve = valueExpr(x,e,pin);//传入x1 or x2
    }
    else{
        ve = valueExpr(x,nullptr,pin);//传入左式
    }
    std::shared_ptr<PhiExpression> vpf = valuePhiFunc(ve,pin);
    if (findexp_insert(pout,ve,dynamic_cast<Value*>(x)) || findvpf_insert(pout,vpf,dynamic_cast<Value*>(x))){
        ;
    }  
    else{
        //POUTs = POUTs ∪ {vn, x, ve : vpf}
        auto newpar = createCongruenceClass(next_value_number_++);
        newpar->value_expr_ = ve; 
        newpar->members_.insert(x);
        if(vpf) newpar->value_phi_ = vpf;
        pout.insert(newpar);
    }
    // TODO: get different ValueExpr by Instruction::OpID, modify pout
    return pout;
}

//转化为phi语句
shared_ptr<PhiExpression> GVN::valuePhiFunc(shared_ptr<Expression> ve, const partitions &P) {
    // TODO
    //ve is of the form φk(vi1, vj1) ⊕ φk(vi2, vj2)
    if(ve->get_expr_type() == Expression::e_bin){
        if( std::dynamic_pointer_cast<BinaryExpression>(ve)->lhs_->get_expr_type() == Expression::e_phi && 
            std::dynamic_pointer_cast<BinaryExpression>(ve)->rhs_->get_expr_type() == Expression::e_phi){
            //vP= vi1 ⊕ vi2
            auto vlhs = std::dynamic_pointer_cast<BinaryExpression>(ve)->lhs_;
            auto vrhs = std::dynamic_pointer_cast<BinaryExpression>(ve)->rhs_;
            std::shared_ptr<BinaryExpression> vL = vL->create(std::dynamic_pointer_cast<BinaryExpression>(ve)->op_,
                                                                std::dynamic_pointer_cast<PhiExpression>(vlhs)->lhs_,
                                                                std::dynamic_pointer_cast<PhiExpression>(vrhs)->lhs_);
            std::shared_ptr<Expression> vi = getVN(P,vL);
            if(vi == nullptr){
                vi = valuePhiFunc(vL, P);
            }
            std::shared_ptr<BinaryExpression> vR = vR->create(std::dynamic_pointer_cast<BinaryExpression>(ve)->op_,
                                                                std::dynamic_pointer_cast<PhiExpression>(vlhs)->rhs_,
                                                                std::dynamic_pointer_cast<PhiExpression>(vrhs)->rhs_);
            std::shared_ptr<Expression> vj = getVN(P,vR);
            if(vj == nullptr){
                vj = valuePhiFunc(vR, P);
            }
            std::shared_ptr<PhiExpression> vr = vr->create(vi,vj);
            if(vi && vj) return vr;
        }
    }
    return nullptr;
}

shared_ptr<Expression> GVN::getVN(const partitions &pout, shared_ptr<Expression> ve) {
    // TODO: return what?
    for (auto it = pout.begin(); it != pout.end(); it++)
        if ((*it)->value_expr_ and *(*it)->value_expr_ == *ve)
            return (*it) -> value_expr_;/*????????????????????????????????????*/
    return nullptr;
}

void GVN::initPerFunction() {
    next_value_number_ = 1;
    pin_.clear();
    pout_.clear();
}

void GVN::replace_cc_members() {
    for (auto &[_bb, part] : pout_) {
        auto bb = _bb; // workaround: structured bindings can't be captured in C++17
        for (auto &cc : part) {
            if (cc->index_ == 0)
                continue;
            // if you are planning to do constant propagation, leaders should be set to constant at some point
            for (auto &member : cc->members_) {
                bool member_is_phi = dynamic_cast<PhiInst *>(member);
                bool value_phi = cc->value_phi_ != nullptr;
                if (member != cc->leader_ and (value_phi or !member_is_phi)) {
                    // only replace the members if users are in the same block as bb
                    member->replace_use_with_when(cc->leader_, [bb](User *user) {
                        if (auto instr = dynamic_cast<Instruction *>(user)) {
                            auto parent = instr->get_parent();
                            auto &bb_pre = parent->get_pre_basic_blocks();
                            if (instr->is_phi()) // as copy stmt, the phi belongs to this block
                                return std::find(bb_pre.begin(), bb_pre.end(), bb) != bb_pre.end();
                            else
                                return parent == bb;
                        }
                        return false;
                    });
                }
            }
        }
    }
    return;
}

// top-level function, done for you
void GVN::run() {
    std::ofstream gvn_json;
    if (dump_json_) {
        gvn_json.open("gvn.json", std::ios::out);
        gvn_json << "[";
    }

    folder_ = std::make_unique<ConstFolder>(m_);
    func_info_ = std::make_unique<FuncInfo>(m_);
    func_info_->run();
    dce_ = std::make_unique<DeadCode>(m_);
    dce_->run(); // let dce take care of some dead phis with undef

    for (auto &f : m_->get_functions()) {
        if (f.get_basic_blocks().empty())
            continue;
        func_ = &f;
        initPerFunction();
        LOG_INFO << "Processing " << f.get_name();
        detectEquivalences();
        LOG_INFO << "===============pin=========================\n";
        for (auto &[bb, part] : pin_) {
            LOG_INFO << "\n===============bb: " << bb->get_name() << "=========================\npartitionIn: ";
            for (auto &cc : part)
                LOG_INFO << utils::print_congruence_class(*cc);
        }
        LOG_INFO << "\n===============pout=========================\n";
        for (auto &[bb, part] : pout_) {
            LOG_INFO << "\n=====bb: " << bb->get_name() << "=====\npartitionOut: ";
            for (auto &cc : part)
                LOG_INFO << utils::print_congruence_class(*cc);
        }
        if (dump_json_) {
            gvn_json << "{\n\"function\": ";
            gvn_json << "\"" << f.get_name() << "\", ";
            gvn_json << "\n\"pout\": " << utils::dump_bb2partition(pout_);
            gvn_json << "},";
        }
        replace_cc_members(); // don't delete instructions, just replace them
    }
    dce_->run(); // let dce do that for us
    if (dump_json_)
        gvn_json << "]";
}

template <typename T>
static bool equiv_as(const Expression &lhs, const Expression &rhs) {
    // we use static_cast because we are very sure that both operands are actually T, not other types.
    return static_cast<const T *>(&lhs)->equiv(static_cast<const T *>(&rhs));
}

bool GVNExpression::operator==(const Expression &lhs, const Expression &rhs) {
    if (lhs.get_expr_type() != rhs.get_expr_type())
        return false;
    switch (lhs.get_expr_type()) {
    case Expression::e_constant: return equiv_as<ConstantExpression>(lhs, rhs);
    case Expression::e_bin: return equiv_as<BinaryExpression>(lhs, rhs);
    case Expression::e_phi: return equiv_as<PhiExpression>(lhs, rhs);
    }
}

bool GVNExpression::operator==(const shared_ptr<Expression> &lhs, const shared_ptr<Expression> &rhs) {
    if (lhs == nullptr and rhs == nullptr) // is the nullptr check necessary here?
        return true;
    return lhs and rhs and *lhs == *rhs;
}

GVN::partitions GVN::clone(const partitions &p) {
    partitions data;
    for (auto &cc : p) {
        data.insert(std::make_shared<CongruenceClass>(*cc));
    }
    return data;
}

bool operator==(const GVN::partitions &p1, const GVN::partitions &p2) {
    // TODO: how to compare partitions?
    auto it1 = p1.begin();
    auto it2 = p2.begin();
    for(;;it1++,it2++){
        if(it1 == p1.end() && it2 != p2.end()) return false;
        else if(it1 != p1.end() && it2 == p2.end()) return false;
        else if(it1 == p1.end() && it2 == p2.end()) return true;
        else{
            if(*(*it1) == *(*it2)) continue;
            else return false;
        }
    }
    return false;
}

bool CongruenceClass::operator==(const CongruenceClass &other) const {
    // TODO: which fields need to be compared?
    auto it1 = this->members_.begin();
    auto it2 = other.members_.begin();
    for(;;it1++,it2++){
        if(it1 == this->members_.end() && it2 != other.members_.end()) return false;
        else if(it1 != this->members_.end() && it2 == other.members_.end()) return false;
        else if(it1 == this->members_.end() && it2 == other.members_.end()) return true;
        else{
            if(*it1 == *it2) continue;
            else return false;
        }
    }
    
}
