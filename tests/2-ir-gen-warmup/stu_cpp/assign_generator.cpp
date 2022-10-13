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

  // main函数
  auto mainFun = Function::create(FunctionType::get(Int32Type, {}),
                                  "main", module); // 创建并返回函数，参数依次是待创建函数类型 ty，函数名字 name (不可为空)，函数所属的模块 parent

  auto bb = BasicBlock::create(module, "entry", mainFun);

  // BasicBlock的名字在生成中无所谓,但是可以方便阅读
  builder->set_insert_point(bb);

  auto *arraytype = ArrayType::get(Int32Type, 10);
  //auto *arraytype = Type::get_array_type(Int32Type,10);
  auto a_alloca = builder->create_alloca(arraytype);//创建int a[10]


  auto x0GEP = builder->create_gep(a_alloca, {CONST_INT(0), CONST_INT(0)});  // GEP: 这里为什么是{0, 0}呢? (实验报告相关)
  builder->create_store(CONST_INT(10), x0GEP);//  a[0] = 10

  auto x0load =builder->create_load(x0GEP);//取出x0的值
  auto mul = builder->create_imul(CONST_INT(2), x0load);  //   a[0] * 2
  auto x1GEP = builder->create_gep(a_alloca, {CONST_INT(0), CONST_INT(1)});//获得x1地址
  builder->create_store(mul, x1GEP);
  auto x01load =builder->create_load(x1GEP);

  builder->create_ret(x01load);
  std::cout << module->print();
  delete module;
  return 0;
}
