#include "BasicBlock.h"
#include "Constant.h"
#include "Function.h"
#include "IRBuilder.h"
#include "Module.h"
#include "Type.h"

#include <iostream>
#include <memory>
#define DEBUG
#ifdef DEBUG // 用于调试信息,大家可以在编译过程中通过" -DDEBUG"来开启这一选项
#define DEBUG_OUTPUT std::cout << __LINE__ << std::endl; // 输出行号的简单示例
#else
#define DEBUG_OUTPUT
#endif

#define CONST_INT(num) ConstantInt::get(num, module)

#define CONST_FP(num) ConstantFP::get(num, module) // 得到常数值的表示,方便后面多次用到

int main() {
  auto module = new Module("Cminus code");  // module name是什么无关紧要
  auto builder = new IRBuilder(nullptr, module);

  Type *Int32Type = Type::get_int32_type(module);
  Type *floatType = Type::get_float_type(module);

  // main函数
  auto mainFun = Function::create(FunctionType::get(Int32Type, {}),
                                  "main", module); // 创建并返回函数，参数依次是待创建函数类型 ty，函数名字 name (不可为空)，函数所属的模块 parent

  auto bb = BasicBlock::create(module, "entry", mainFun);

  // BasicBlock的名字在生成中无所谓,但是可以方便阅读
  builder->set_insert_point(bb);


  auto a_alloca = builder->create_alloca(floatType);//创建float a


  auto x0GEP = builder->create_gep(a_alloca, {CONST_INT(0)});  
  builder->create_store(CONST_FP(5.555), x0GEP);//  a = 5.555

  auto x0Load =builder->create_load(x0GEP);//取出a的值
  auto fcmp = builder->create_fcmp_gt(x0Load, CONST_FP(1.0));  // a和1的比较,注意ICMPEQ
  auto trueBB = BasicBlock::create(module, "trueBB", mainFun);    // true分支
  auto falseBB = BasicBlock::create(module, "falseBB", mainFun);  // false分支
  auto br = builder->create_cond_br(fcmp, trueBB, falseBB);  // 条件BR
  builder->set_insert_point(trueBB);  // if true; 分支的开始需要SetInsertPoint设置
  builder->create_ret({CONST_INT(233)});
  builder->set_insert_point(falseBB);  // if false
  builder->create_ret({CONST_INT(0)});

  std::cout << module->print();
  delete module;
  return 0;
}
