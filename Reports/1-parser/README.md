# lab1 实验报告
PB20111694 毛陈诚

## 实验要求

1. 了解Cminus-f词法（关键字，专用符号，标识符ID和整数NUM，注释）与语法规则。
2. 了解`Flex`用法以及lex的语法，对输入的字符串用正则表达式进行分析
3. 了解`Bison`用法，及其如何将LALR 文法转换成可编译的 C 代码
4. 了解如何让` bison` 和 `flex`协同工作——重点是如何维护解析器状态、`YYSTYPE` 和头文件的生成
5. 基于 `flex` 的词法分析器和基于 `bison` 的语法分析器，从无到有完成一个完整的 Cminus-f 解析器

## 实验难点

1. 利用正则表达式表示输入字符串，尤其是对于注释的表示
2. 理解语法树的生成过程与相应的数据结构，并依据语法树填写union类与利用node函数构建树节点
3. 了解`Flex`的用法，包括模式与对应动作，理解yytext与yyleng的用法。
4. 理解`bison`解析规则，补全token，理解‘’$$“,”$1“,”$2“…的含义，理解`YYSTYPE`以及利用`YYSTYPE`对每个节点赋对应的语义值。

## 实验设计

#### 词法分析器

> 用于解析输入c语言并生成token stream，利用\build\syntax_analyzer.h定义的符号，对token进行操作，包括设置pos_start,pos_end变化，调用pass_node创建相应的树节点，返回对应的token值。

#### 语法分析器

1. 补全union

   根据每个模式的动作是调用node创建树节点，而node函数的返回值与第三个及以后参数数据类型为树节点的指针，因此union的数据为struct _syntax_tree_node *。

2. 补全`%token`与`%type`

   利用词法分析器中定义的`token`与Cminus-f词法补全

3. 根据C-minus生产式完成模式-动作填写

   理解语法树构建过程，利用node函数与’$$“,”$1“,”$2“…传递节点的值

## 实验结果验证

词法分析：

![image-20220928095239232](D:\360MoveData\Users\admin\Desktop\coursework\compiler\lab\lab1\2022fall-compiler_cminus\Reports\1-parser\image-20220928095239232.png)

语法树生成：

```
root@70c6db0997ce:/labs/lab1/2022fall-compiler_cminus/build# ./parser ../tests/parser/easy/id.cminus
>--+ program
|  >--+ declaration-list
|  |  >--+ declaration-list
|  |  |  >--+ declaration-list
|  |  |  |  >--+ declaration
|  |  |  |  |  >--+ var-declaration
|  |  |  |  |  |  >--+ type-specifier
|  |  |  |  |  |  |  >--* int
|  |  |  |  |  |  >--* a
|  |  |  |  |  |  >--* ;
|  |  |  >--+ declaration
|  |  |  |  >--+ fun-declaration
|  |  |  |  |  >--+ type-specifier
|  |  |  |  |  |  >--* int
|  |  |  |  |  >--* f
|  |  |  |  |  >--* (
|  |  |  |  |  >--+ params
|  |  |  |  |  |  >--* void
|  |  |  |  |  >--* )
|  |  |  |  |  >--+ compound-stmt
|  |  |  |  |  |  >--* {
|  |  |  |  |  |  >--+ local-declarations
|  |  |  |  |  |  |  >--* epsilon
|  |  |  |  |  |  >--+ statement-list
|  |  |  |  |  |  |  >--* epsilon
|  |  |  |  |  |  >--* }
|  |  >--+ declaration
|  |  |  >--+ fun-declaration
|  |  |  |  >--+ type-specifier
|  |  |  |  |  >--* int
|  |  |  |  >--* g
|  |  |  |  >--* (
|  |  |  |  >--+ params
|  |  |  |  |  >--* void
|  |  |  |  >--* )
|  |  |  |  >--+ compound-stmt
|  |  |  |  |  >--* {
|  |  |  |  |  >--+ local-declarations
|  |  |  |  |  |  >--* epsilon
|  |  |  |  |  >--+ statement-list
|  |  |  |  |  |  >--* epsilon
|  |  |  |  |  >--* }
```

easy：

![image-20220928075237240](D:\360MoveData\Users\admin\Desktop\coursework\compiler\lab\lab1\2022fall-compiler_cminus\Reports\1-parser\image-20220928075237240.png)

normal：

![image-20220928075310495](D:\360MoveData\Users\admin\Desktop\coursework\compiler\lab\lab1\2022fall-compiler_cminus\Reports\1-parser\image-20220928075310495.png)

hard：

![image-20220928075334391](D:\360MoveData\Users\admin\Desktop\coursework\compiler\lab\lab1\2022fall-compiler_cminus\Reports\1-parser\image-20220928075334391.png)

## 实验反馈

1. 可以增加对正则表达式的介绍，lex的正则表达式与python或C++语言上在很多方面语法并不匹配（比如（我也不太确定）lex用`？`好像不能实现非贪婪匹配），提供的英文文档并不完备，且学习成本较大。
2. 整体难度不大，就是填生成式时好麻烦，期间好多标识符打错了。。。注意点还有Unix与Window系统下的换行符是不同的，前者是`\n`,后者是`\n\r`。明白这个很有收获!!!
