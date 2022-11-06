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
    if(node.type == TYPE_INT)
    	tmp_val = CONST_INT(node.i_val);
	else
		tmp_val = CONST_FP(node.f_val);
}

void CminusfBuilder::visit(ASTVarDeclaration &node) {
    //!TODO: This function is empty now.
    // Add some code here.变量声明
    //全局变量与局部变量
    //var-declaration ->type-specifier ID; | type-specifier ID[INTEGER];
    Type *var_type;
    if (node.type == TYPE_INT)
        var_type = INT32_T;
    else if (node.type == TYPE_FLOAT)
        var_type = FLOAT_T;
    if(scope.in_global()){//全局变量
        if(node.num != NULL){//数组
            auto arrayType = ArrayType::get(var_type, node.num->i_val);
            auto initializer = ConstantZero::get(var_type, module.get());
            auto global_var = GlobalVariable::create(node.id, module.get(), arrayType, false, initializer);// 参数解释：  名字name，所属module，全局变量类型type，是否是常量定义（cminus中没有常量概念，应全都是false），初始化常量(ConstantZero类)
            scope.push(node.id,global_var);
        }
        else{
            auto initializer = ConstantZero::get(var_type, module.get());
            auto global_var = GlobalVariable::create(node.id,module.get(),var_type,false,initializer);
            scope.push(node.id,global_var);
        }
    }
    else{//局部变量
        if(node.num != NULL){
            auto *arraytype = ArrayType::get(var_type, node.num->i_val);
            auto a_alloca = builder->create_alloca(arraytype);//创建数组
            std::string name = node.id;//变量名
            scope.push(name,a_alloca);   
        }
        else{
            auto a_alloc = builder->create_alloca(var_type);
            std::string name = node.id;//变量名
            scope.push(name,a_alloc);   
        }
    }
}

void CminusfBuilder::visit(ASTFunDeclaration &node) {
    //fun-declaration->type-specifier ID (params) compound_stmt
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
        args.push_back(*arg);//存储函数参数列表
    }
    for (int i = 0; i < node.params.size(); ++i) {
        //!TODO: You need to deal with params
        // and store them in the scope.
        Value * a_alloc;
        if(node.params[i]->isarray){//数组
            if(node.params[i]->type == TYPE_INT)
                a_alloc = builder->create_alloca(INT32PTR_T);
            else if(node.params[i]->type = TYPE_FLOAT)
                a_alloc = builder->create_alloca(FLOATPTR_T);  
            builder->create_store(args[i],a_alloc) ;
            std::string name = node.params[i]->id;//变量名
            scope.push(name,a_alloc);          
        }
        else{
            if(node.params[i]->type == TYPE_INT)
                a_alloc = builder->create_alloca(INT32_T);
            else if(node.params[i]->type = TYPE_FLOAT)
                a_alloc = builder->create_alloca(FLOAT_T);  
            builder->create_store(args[i],a_alloc);
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
    //param->type-specified ID | TYPE-specified ID[]
    //fun-declaration实现过了
}

void CminusfBuilder::visit(ASTCompoundStmt &node) {
    //!TODO: This function is not complete.
    // You may need to add some code here
    // to deal with complex statements. 
    //compound-stmt->{local-declarations statement-list}
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
    //exprression-stmt->expression;|;
    if(node.expression == nullptr)
        return;
    else{
        node.expression->accept(*this);
    }
}

void CminusfBuilder::visit(ASTSelectionStmt &node) {//expression,if_statement,else_statement
    //!TODO: This function is empty now.
    // Add some code here.
    // $selection-stmt->if(expression) statement if(expression) statement else statement
    node.expression->accept(*this);
    if (tmp_val->get_type() == INT32PTR_T || tmp_val->get_type() == FLOATPTR_T)
	{
 		tmp_val = builder->create_load(tmp_val);
 	}
    //判断表达式是否为真
    if (tmp_val->get_type() == INT32_T)
 	{
		tmp_val = builder->create_icmp_ne(tmp_val, CONST_INT(0));
	}
	if (tmp_val->get_type() == FLOAT_T)
	{
		tmp_val = builder->create_fcmp_ne(tmp_val, CONST_FP(0));
	}
    auto iftrue = BasicBlock::create(module.get(), "", cur_fun);
    auto next = BasicBlock::create(module.get(), "", cur_fun);
    if(node.else_statement != NULL){//有else语句
        auto iffalse = BasicBlock::create(module.get(), "", cur_fun);
        auto br = builder -> create_cond_br(tmp_val,iftrue,iffalse);
        builder->set_insert_point(iftrue);
        node.if_statement->accept(*this);
        if (builder->get_insert_block()->get_terminator() == nullptr)
		    builder->create_br(next);
        builder->set_insert_point(iffalse);
        node.else_statement->accept(*this);
    }
    else{
        builder->set_insert_point(iftrue);
        node.if_statement->accept(*this);
        builder->set_insert_point(next);
    } 
}

void CminusfBuilder::visit(ASTIterationStmt &node) {
    //!TODO: This function is empty now.
    // Add some code here.
    auto loop_enter = BasicBlock::create(module.get(), "", cur_fun);
    builder->set_insert_point(loop_enter);
    auto whiletrue = BasicBlock::create(module.get(), "", cur_fun);
    auto whilefalse = BasicBlock::create(module.get(), "", cur_fun);
    node.expression->accept(*this);
    if (tmp_val->get_type() == INT32PTR_T || tmp_val->get_type() == FLOATPTR_T)
	{
 		tmp_val = builder->create_load(tmp_val);
    }
    if (tmp_val->get_type() == INT32_T)
 	{
		tmp_val = builder->create_icmp_ne(tmp_val, CONST_INT(0));
	}
	if (tmp_val->get_type() == FLOAT_T)
	{
		tmp_val = builder->create_fcmp_ne(tmp_val, CONST_FP(0));
	}
    builder->create_cond_br(tmp_val,whiletrue,whilefalse);
    builder->set_insert_point(whiletrue);
    node.statement->accept(*this);
    builder->create_br(loop_enter);
    builder->set_insert_point(whilefalse);
}

void CminusfBuilder::visit(ASTReturnStmt &node) {
    if (node.expression == nullptr) {
        builder->create_void_ret();
    } else {
        //!TODO: The given code is incomplete.
        // You need to solve other return cases (e.g. return an integer).
        node.expression->accept(*this);
        auto return_type = builder->get_insert_block()->get_parent()->get_function_type();//函数返回值类型
        if(return_type == INT32_T){
            if(tmp_val->get_type() == FLOAT_T){
                tmp_val = builder->create_fptosi(tmp_val,INT32_T);
            }
            builder->create_ret(tmp_val);
        }
        else if(return_type== FLOAT_T){
            node.expression->accept(*this);
            if(tmp_val->get_type() == INT32_T){
                tmp_val = builder->create_fptosi(tmp_val,FLOAT_T);
            }
            builder->create_ret(tmp_val);
        }
    }
}

void CminusfBuilder::visit(ASTVar &node) {
    //!TODO: This function is empty now.
    // Add some code here.
    //var→ID ∣ ID [ expression] 
    auto var = scope.find(node.id);
    auto isarray = var->get_type()->get_pointer_element_type()->is_array_type();//for debug
    if(node.expression == nullptr){//变量为int、float类型
        if(!isarray)
            tmp_val = var;
    }
    else{//数组类型
        node.expression->accept(*this);//tmp_val此时存储偏移量
        auto array_type = var->get_type()->get_pointer_element_type();//数组元素类型
        if (tmp_val->get_type() == FLOAT_T)//对数组下表强制转换为整型
 		{
 			tmp_val = builder->create_fptosi(tmp_val, INT32_T);
 		}
        /*to do 数组下标正负判断*/
        tmp_val = builder->create_gep(tmp_val, {CONST_INT(0),tmp_val}); 
    }
}

void CminusfBuilder::visit(ASTAssignExpression &node) {
    //!TODO: This function is empty now.
    // Add some code here.
    // var = expression
    //  
    Value *addr;
    node.var->accept(*this);
    addr = tmp_val;
    auto type = addr->get_type();//for debug
    node.expression->accept(*this);
    auto rval = tmp_val;
    if(addr->get_type() != rval->get_type()){
        if(rval->get_type() == INT32_T){
            rval = builder->create_sitofp(rval,FLOAT_T);
            builder->create_store(rval,addr); 
        }
        else if(rval->get_type() == FLOAT_T){
            rval = builder->create_fptosi(rval,INT32_T);
            builder->create_store(rval,addr); 
        }
    }
}

void CminusfBuilder::visit(ASTSimpleExpression &node) {
    //!TODO: This function is empty now.
    // Add some code here.
    //simple-expression->additive-expression relop additive-expression | additive-expression
    Value* cmp;
    node.additive_expression_l->accept(*this);
    if (node.additive_expression_r != nullptr){
        auto lval = tmp_val;
        Value *lvalue;
        if(lval->get_type() == INT32PTR_T || lval->get_type() == FLOATPTR_T){
            lvalue = builder->create_load(tmp_val);//如果是数组，从地址load入数据
        }
        else{
            lvalue = lval;
        }
        node.additive_expression_r->accept(*this);
        auto rval = tmp_val;
        Value *rvalue;
        if(rval->get_type() == INT32PTR_T || rval->get_type() == FLOATPTR_T){
            rvalue = builder->create_load(tmp_val);//如果是数组，从地址load入数据
        }
        else{
            rvalue = rval;
        }
        //类型转换
        if(lvalue->get_type() != rvalue->get_type()){
            if(lvalue->get_type() == INT32_T)//lval是int,r是float,把l转换为float
                lvalue = builder->create_sitofp(lvalue,FLOAT_T);
            else if(lvalue->get_type() == FLOAT_T)//
                rvalue = builder->create_sitofp(rvalue,FLOAT_T);
        }
        if(node.op == OP_LE){  
            if(lvalue->get_type() == INT32_T)
                cmp = builder->create_icmp_le(lvalue,rvalue);
            else if(lvalue->get_type() == FLOAT_T)
                cmp = builder->create_icmp_le(lvalue,rvalue);
        }
        else if(node.op == OP_LT){
            if(lvalue->get_type() == INT32_T)
                cmp = builder->create_icmp_lt(lvalue,rvalue);
            else if(lvalue->get_type() == FLOAT_T)
                cmp = builder->create_icmp_lt(lvalue,rvalue);
        }
        else if(node.op == OP_GT){
            if(lvalue->get_type() == INT32_T)
                cmp = builder->create_icmp_gt(lvalue,rvalue);
            else if(lvalue->get_type() == FLOAT_T)
                cmp = builder->create_icmp_gt(lvalue,rvalue);
        }
        else if(node.op == OP_GE){
            if(lvalue->get_type() == INT32_T)
                cmp = builder->create_icmp_ge(lvalue,rvalue);
            else if(lvalue->get_type() == FLOAT_T)
                cmp = builder->create_icmp_ge(lvalue,rvalue);
        }
        else if(node.op == OP_EQ){
            if(lvalue->get_type() == INT32_T)
                cmp = builder->create_icmp_eq(lvalue,rvalue);
            else if(lvalue->get_type() == FLOAT_T)
                cmp = builder->create_icmp_eq(lvalue,rvalue);
        }
        else if(node.op == OP_NEQ){
            if(lvalue->get_type() == INT32_T)
                cmp = builder->create_icmp_ne(lvalue,rvalue);
            else if(lvalue->get_type() == FLOAT_T)
                cmp = builder->create_icmp_ne(lvalue,rvalue);
        }
        tmp_val = cmp;
    }
}

void CminusfBuilder::visit(ASTAdditiveExpression &node) {
    //!TODO: This function is empty now.
    // Add some code here.
    //additive-expression relop term | term
    node.term->accept(*this);
    if(node.additive_expression){
        auto term_val = tmp_val;
        Value *lvalue;
        if(term_val->get_type() == INT32PTR_T || term_val->get_type() == FLOATPTR_T){
            lvalue = builder->create_load(tmp_val);//如果是数组，从地址load入数据
        }
        else{
            lvalue = term_val;
        }
        node.additive_expression->accept(*this);
        auto expr_val = tmp_val;
        Value *rvalue;
        if(expr_val->get_type() == INT32PTR_T || expr_val->get_type() == FLOATPTR_T){
            rvalue = builder->create_load(expr_val);//如果是数组，从地址load入数据
        }
        else{
            rvalue = expr_val;
        }
        //类型转换
        if(lvalue->get_type() != rvalue->get_type()){
            if(lvalue->get_type() == INT32_T)//lval是int,r是float,把l转换为float
                lvalue = builder->create_sitofp(lvalue,FLOAT_T);
            else if(lvalue->get_type() == FLOAT_T)//
                rvalue = builder->create_sitofp(rvalue,FLOAT_T);
        }
        Value *result;
        if(node.op == OP_PLUS){
            if(lvalue->get_type() == INT32_T)
                result = builder->create_iadd(lvalue, rvalue);
            else if(lvalue->get_type() == FLOAT_T)
                result = builder->create_fadd(lvalue, rvalue);
        }
        else if(node.op == OP_MINUS){
            if(lvalue->get_type() == INT32_T)
                result = builder->create_isub(lvalue, rvalue);
            else if(lvalue->get_type() == FLOAT_T)
                result = builder->create_fsub(lvalue, rvalue);
        }
        tmp_val = result;
    }
}

void CminusfBuilder::visit(ASTTerm &node) {
    //!TODO: This function is empty now.
    // Add some code here.
    //term->term mulop factor | factor
    node.factor->accept(*this);
    if (node.term){
        auto term_val = tmp_val;
        Value *lvalue;
        if(term_val->get_type() == INT32PTR_T || term_val->get_type() == FLOATPTR_T){
            lvalue = builder->create_load(tmp_val);//如果是数组，从地址load入数据
        }
        else{
            lvalue = term_val;
        }
        node.factor->accept(*this);
        auto factor_val = tmp_val;
        Value *rvalue;
        if(factor_val->get_type() == INT32PTR_T || factor_val->get_type() == FLOATPTR_T){
            rvalue = builder->create_load(factor_val);//如果是数组，从地址load入数据
        }
        else{
            rvalue = factor_val;
        }
        //类型转换
        if(lvalue->get_type() != rvalue->get_type()){
            if(lvalue->get_type() == INT32_T)//lval是int,r是float,把l转换为float
                lvalue = builder->create_sitofp(lvalue,FLOAT_T);
            else if(lvalue->get_type() == FLOAT_T)//
                rvalue = builder->create_sitofp(rvalue,FLOAT_T);
        }
        Value *result;
        if(node.op == OP_MUL){
            if(lvalue->get_type() == INT32_T)
                result = builder->create_imul(lvalue, rvalue);
            else if(lvalue->get_type() == FLOAT_T)
                result = builder->create_fmul(lvalue, rvalue);
        }
        else if(node.op == OP_DIV){
            if(lvalue->get_type() == INT32_T)
                result = builder->create_isdiv(lvalue, rvalue);
            else if(lvalue->get_type() == FLOAT_T)
                result = builder->create_fdiv(lvalue, rvalue);
        }
        tmp_val = result;
    }
}

void CminusfBuilder::visit(ASTCall &node) {
    //!TODO: This function is empty now.
    // Add some code here.
    //call->ID(args)
    // std::string id;
    // std::vector<std::shared_ptr<ASTExpression>> args;
    auto func = static_cast<Function *>(scope.find(node.id));
    auto func_type = func->get_function_type();
    std::vector<Type *> params;
    for (auto arg = func_type->param_begin(); arg != func_type->param_end(); arg++)
    {
 		params.push_back(*arg);//参数类型，构建函数由param-lists传入
    }
    int i = 0;
    std::vector<Value *> funArgs;
    //传入参数与实际参数类型转换
    for (auto &arg : node.args){
        arg->accept(*this);
        if(params[i] == INT32_T){
            if(tmp_val->get_type() == INT32_T){
                    tmp_val = builder->create_fptosi(tmp_val, INT32_T);
            }
            else if (tmp_val->get_type() == INT32PTR_T)
            {
                tmp_val = builder->create_load(tmp_val);
            }
            else if(tmp_val->get_type() == FLOATPTR_T){
                tmp_val = builder->create_load(tmp_val);
                tmp_val = builder->create_fptosi(tmp_val, INT32_T);
            }
        }
        else if(params[i] == FLOAT_T){
            if(tmp_val->get_type() == INT32_T){
                tmp_val = builder->create_sitofp(tmp_val, FLOAT_T);
            }
            else if (tmp_val->get_type() == INT32PTR_T)
            {
                tmp_val = builder->create_load(tmp_val);
                tmp_val = builder->create_sitofp(tmp_val, FLOAT_T);
            }
            else if(tmp_val->get_type() == FLOATPTR_T){
                tmp_val = builder->create_load(tmp_val);
            }
        }
        funArgs.push_back(tmp_val);
        i++;
    }
    tmp_val = builder->create_call(func, funArgs);
}


/*
        Value * a_alloc;
        if(node.params[i]->isarray){//数组
            if(node.params[i]->type == TYPE_INT)
                a_alloc = builder->create_alloca(INT32PTR_T);
            else if(node.params[i]->type = TYPE_FLOAT)
                a_alloc = builder->create_alloca(FLOATPTR_T);  
            builder->create_store(args[i],a_alloc) ;
            std::string name = node.params[i]->id;//变量名
            scope.push(name,a_alloc);          
        }
        else{
            if(node.params[i]->type == TYPE_INT)
                a_alloc = builder->create_alloca(INT32_T);
            else if(node.params[i]->type = TYPE_FLOAT)
                a_alloc = builder->create_alloca(FLOAT_T);  
            builder->create_store(args[i],a_alloc);
            std::string name = node.params[i]->id;//变量名
            scope.push(name,a_alloc); 
        }
    }
*/