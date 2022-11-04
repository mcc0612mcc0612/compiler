/*
 * 声明：本代码为 2022 秋 中国科大编译原理（李诚）课程实验参考实现。
 * 请不要以任何方式，将本代码上传到可以公开访问的站点或仓库
 */

#include "cminusf_builder.hpp"

#define CONST_FP(num) ConstantFP::get((float)num, module.get())
#define CONST_INT(num) ConstantInt::get(num, module.get())


// TODO: Global Variable Declarations 
// You can define global variables here
// to store state. You can expand these
// definitions if you need to.
Value *tmp_val = nullptr;
// function that is being built
Function *cur_fun = nullptr;

// types
Type *VOID_T;
Type *INT1_T;
Type *INT32_T;
Type *INT32PTR_T;
Type *FLOAT_T;
Type *FLOATPTR_T;

/*
 * use CMinusfBuilder::Scope to construct scopes
 * scope.enter: enter a new scope
 * scope.exit: exit current scope
 * scope.push: add a new binding to current scope
 * scope.find: find and return the value bound to the name
 */

void CminusfBuilder::visit(ASTProgram &node) {
    VOID_T = Type::get_void_type(module.get());
    INT1_T = Type::get_int1_type(module.get());
    INT32_T = Type::get_int32_type(module.get());
    INT32PTR_T = Type::get_int32_ptr_type(module.get());
    FLOAT_T = Type::get_float_type(module.get());
    FLOATPTR_T = Type::get_float_ptr_type(module.get());

    for (auto decl : node.declarations) {// program ->declaration-list
        decl->accept(*this);
    }
}

void CminusfBuilder::visit(ASTNum &node) {
    //!TODO: This function is empty now.
    // Add some code here.
    
}

void CminusfBuilder::visit(ASTVarDeclaration &node) {
    //!TODO: This function is empty now.
    // Add some code here.变量声明
    //全局变量与局部变量
    Type *var_type;
    if (node.type == TYPE_INT)
        var_type = INT32_T;
    else if (node.type == TYPE_FLOAT)
        var_type = FLOAT_T;
    if(scope.in_global){//全局变量
        if(node.num != NULL){//数组
            auto arrayType = ArrayType::get(var_type, node.num->i_val);
            auto initializer = ConstantZero::get(var_type, module.get());
            auto global_var = GlobalVariable::create(node.id, module, arrayType, false, initializer);// 参数解释：  名字name，所属module，全局变量类型type，是否是常量定义（cminus中没有常量概念，应全都是false），初始化常量(ConstantZero类)
            scope.push(node.id,global_var);
        }
        else{
            auto initializer = ConstantZero::get(var_type, module.get());
            auto global_var = GlobalVariable::create(node.id,module,var_type,false,initializer);
            scope.push(node.id,global_var);
        }
    }
    else{//局部变量
        if(node.num != NULL){
            auto *arraytype = ArrayType::get(var_type, node.num->i_val);
            auto a_alloca = builder->create_alloca(arraytype);//创建数组
            std::string name = node.id;//变量名
            scope.push(name,a_alloc);   
        }
        else{
            auto a_alloc = builder->create_alloca(var_type);
            std::string name = node.id;//变量名
            scope.push(name,a_alloc);   
        }
    }
}

void CminusfBuilder::visit(ASTFunDeclaration &node) {
    FunctionType *fun_type;
    Type *ret_type;
    std::vector<Type *> param_types;
/*-----------------返回值类型---------------*/
    if (node.type == TYPE_INT)
        ret_type = INT32_T;
    else if (node.type == TYPE_FLOAT)
        ret_type = FLOAT_T;
    else
        ret_type = VOID_T;
/*-----------------传入参数类型---------------*/
    for (auto &param : node.params) {
        //!TODO: Please accomplish param_types.
        if(param->isarray){//数组
            if (param->type == TYPE_INT)
                param_types.push_back(INT32PTR_T);
            else if (param->type == TYPE_FLOAT)
                param_types.push_back(FLOATPTR_T);
        }
        else{
            if (param->type == TYPE_INT)
                param_types.push_back(INT32_T);
            else if (param->type == TYPE_FLOAT)
                param_types.push_back(FLOAT_T);
        }
    }
/*-----------------构建函数---------------*/
    fun_type = FunctionType::get(ret_type, param_types);//函数类型：返回值类型+参数类型
    auto fun = Function::create(fun_type, node.id, module.get());//函数类型+函数名+所属模块
    scope.push(node.id, fun);
    cur_fun = fun;
    auto funBB = BasicBlock::create(module.get(), "entry", fun);
    builder->set_insert_point(funBB);
    scope.enter();
    std::vector<Value *> args;
    for (auto arg = fun->arg_begin(); arg != fun->arg_end(); arg++) {
        args.push_back(*arg);
    }
    for (int i = 0; i < node.params.size(); ++i) {
        //!TODO: You need to deal with params
        // and store them in the scope.
        Value * alloc;
        if(node.params[i]->isarray){
            if(node.params[i]->type = = TYPE_INT)
                a_alloc = builder->create_alloca(INT32PTR_T);
            else if(node.params[i]->type = TYPE_FLOAT)
                a_alloc = builder->create_alloca(FLOATPTR_T);  
            builder->create_store(args[i],a_alloc) 
            std::string name = node.params[i]->id;//变量名
            scope.push(name,a_alloc);          
        }
        else{
            if(node.params[i]->type = = TYPE_INT)
                a_alloc = builder->create_alloca(INT32_T);
            else if(node.params[i]->type = TYPE_FLOAT)
                a_alloc = builder->create_alloca(FLOAT_T);  
            builder->create_store(args[i],a_alloc) 
            std::string name = node.params[i]->id;//变量名
            scope.push(name,a_alloc); 
        }
    }
    node.compound_stmt->accept(*this);//进入函数主体
    /*-----------------函数返回---------------*/
    if (builder->get_insert_block()->get_terminator() == nullptr) 
    {
        if (cur_fun->get_return_type()->is_void_type())
            builder->create_void_ret();
        else if (cur_fun->get_return_type()->is_float_type())
            builder->create_ret(CONST_FP(0.));
        else
            builder->create_ret(CONST_INT(0));
    }
    
    scope.exit();
}

void CminusfBuilder::visit(ASTParam &node) {
    //!TODO: This function is empty now.
    // Add some code here.
}

void CminusfBuilder::visit(ASTCompoundStmt &node) {
    //!TODO: This function is not complete.
    // You may need to add some code here
    // to deal with complex statements. 
    //处理局部变量与全局变量冲突
    scope.enter();
    for (auto &decl : node.local_declarations) {
        decl->accept(*this);
    }

    for (auto &stmt : node.statement_list) {
        stmt->accept(*this);
        if (builder->get_insert_block()->get_terminator() != nullptr)
            break;//ret 或 br
    }
    scope.exit();
}

void CminusfBuilder::visit(ASTExpressionStmt &node) {
    //!TODO: This function is empty now.
    // Add some code here.
    if(node.expression == nullptr)
        return;
    else{
        for (auto &expr : node.expression)
            expr->accept(*this);
    }
}

void CminusfBuilder::visit(ASTSelectionStmt &node) {
    //!TODO: This function is empty now.
    // Add some code here.

}

void CminusfBuilder::visit(ASTIterationStmt &node) {
    //!TODO: This function is empty now.
    // Add some code here.
}

void CminusfBuilder::visit(ASTReturnStmt &node) {
    if (node.expression == nullptr) {
        builder->create_void_ret();
    } else {
        //!TODO: The given code is incomplete.
        // You need to solve other return cases (e.g. return an integer).


    }
}

void CminusfBuilder::visit(ASTVar &node) {
    //!TODO: This function is empty now.
    // Add some code here.
    if(node.expression == nullptr){//变量为int、float类型

    }
    else{//数组类型
        node.expression->accept(*this);
    }
}

void CminusfBuilder::visit(ASTAssignExpression &node) {
    //!TODO: This function is empty now.
    // Add some code here.
    //    std::shared_ptr<ASTVar> var;
    //  std::shared_ptr<ASTExpression> expression;
    Value *addr;
    node.var.get()->accept(*this);
    addr == scope.find(node.var.id);//变量地址
    if(value == nullptr)
        neg_idx_except();
    else{
        node.expression->accept(*this);
        node.var->accept(*this);
    }
}

void CminusfBuilder::visit(ASTSimpleExpression &node) {
    //!TODO: This function is empty now.
    // Add some code here.
}

void CminusfBuilder::visit(ASTAdditiveExpression &node) {
    //!TODO: This function is empty now.
    // Add some code here.
}

void CminusfBuilder::visit(ASTTerm &node) {
    //!TODO: This function is empty now.
    // Add some code here.
}

void CminusfBuilder::visit(ASTCall &node) {
    //!TODO: This function is empty now.
    // Add some code here.
}
