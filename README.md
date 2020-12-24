# Intro
Design a language with interpreter, JIT and AoT compilation execution modes. It is supposed to support high performance matrix multiplication through utilizing MLIR.

<p align="center">
  <img src="https://github.com/linsinan1995/compiler/blob/master/other/res.png" img width="350">
</p>
  
# Work List
- Finished
  - Lexer
  - Parser
    - Recursive descend
    - Precedence
  - Execution
    - Interpreter
    - AoT compilation
        - codegen (partial)
  - Code reorganize
    - Visitor pattern for AST
  - Data type
    - matrix
    - fp
    - string
  - Built-in functions register

- To do
  - Error handling
  - MLIR IR emitter
  - Code Gen
    - control flow
    - mutable variable
  - JIT
  - Rich data type
    - indexing
  - code redesign
    - performance bottleneck (llvm::StringRef, llvm::cl ... )


# Lexing

source code:
```
func x(a, b) {
    return a * b
}

x(1,2)
x(x(1,2),3)
```

```
==============TEST: Readme==============
kw_func             	func
k_var               	x
k_open_paren        	(
k_var               	a
k_comma             	,
k_var               	b
k_close_paren       	)
k_open_curly        	{
kw_return           	return
k_var               	a
op_mul              	*
k_var               	b
k_close_curly       	}
k_var               	x
k_open_paren        	(
k_fp                	1
k_comma             	,
k_fp                	2
k_close_paren       	)
k_var               	x
k_open_paren        	(
k_var               	x
k_open_paren        	(
k_fp                	1
k_comma             	,
k_fp                	2
k_close_paren       	)
k_comma             	,
k_fp                	3
k_close_paren       	)
k_EOF
============================================
```


# Parsing

source code:
```
func x(a, b) {
    return a * b
}

x(1,2)
x(x(1,2),3)
```

```
==============  TEST 5: readme  ==============
=========line 1=========
[FUNC]
    func name: x(
        [VAR_EXP] a
    ,
        [VAR_EXP] b
    )
func body:
    [BLOCK]
    empty code block!
return:
    [LHS_EXP]
        [UNARY_EXP]
            [VAR_EXP] a
    [BIN_OP ] op_mul
    [RHS_EXP]
        [UNARY_EXP]
            [VAR_EXP] b
=========line 2=========
[FUNC_CALL]
func : x (
    [UNARY_EXP]
        [FP_EXP] 1
,
    [UNARY_EXP]
        [FP_EXP] 2
)
=========line 3=========
[FUNC_CALL]
func : x (
    [UNARY_EXP]
        [FUNC_CALL]
        func : x (
            [UNARY_EXP]
                [FP_EXP] 1
        ,
            [UNARY_EXP]
                [FP_EXP] 2
        )
,
    [UNARY_EXP]
        [FP_EXP] 3
)

==============================================
```

# Execution

## 1.Interpreter
```
func x(a, b) {
    return a * b
}
x(1,2)
x(x(1,2),3);
```

```
=============  TEST 1: Read me  ==============
2.000000
6.000000
==============================================
```

# Register built-in function from C++
A demo for registering functions

```cpp
#include <iostream>

#include "Interpreter/Builtin_function.hpp"
#include "Interpreter/AST_Interpreter.h"
#include "Parser.h"

using namespace parser_ns;
using namespace runtime_ns;
using namespace register_ns;

// define the function you want to build in
// function should be type
// RT_Value (*) (Runtime*, std::vector<RT_Value>)
RT_Value builtin_println(Runtime* rt, std::vector<RT_Value> args) {
    for (const auto& arg : args) {
        std::cout << arg << "\n";
    }
    if (args.empty()) std::cout << "\n";
    return RT_Value();
}

// generate arbitrary shape of matrix or vector with all elements at value 1.
RT_Value ones(Runtime* rt, std::vector<RT_Value> args) {
    // one dimension vector
    if (args.size() == 1) {
        int len = static_cast<int>(args[0].data.fp);
        Mat mat {std::vector<float> (len, 1), std::vector<int> {len}};
        return RT_Value(std::move(mat));
    }

    // two dimension matrix
    int height = args[0].data._int, width = args[1].data._int;
    Mat mat {
                std::vector<float> (width * height, 1), /* data */
                std::vector<int> { height, width }      /* dim */
            };

    return RT_Value(std::move(mat));
}

// test code
const char *code = "print(123, 42, 52, [1,2,3],\"Hello\", [[1,2,3], [4,5,6]])\n"
                   "print(\"ones(2,3) * ones(3,6)\")\n"
                   "ones(2,3) * ones(3,6)\n"
                   "print(\"ones(15,3) * [0.5, 3.2, 0.4]\")\n"
                   "ones(15,3) * [0.5, 3.2, 0.4]\n"
                   "print(\"ones(4,2)\")\n"
                   "ones(4,2)";


int main() {
    // a helper class for register functions into the global scope during runtime
    builtin_register reg;

    // get ast expression interpreter
    AST_Interpreter interpreter {};

    // get parser
    auto parser = Parser::make_parser(code);

    // add functions into the register helper
    reg.push_back("ones"/*function name*/, ones/*function ptr*/,
                  "print",                 builtin_println);

    // dump functions to runtime
    reg._register(interpreter.rt.get());

    // parsing code to AST
    std::vector<std::shared_ptr<Expression_AST>> v = parser->parse();

    // traversal AST and interpret expression
    for (auto &&expr : v) {
        interpreter.evaluate(*expr);
        if (!interpreter.is_null())
            std::cout << interpreter.val << "\n";
    }
}
```

result
```
123.000000
42.000000
52.000000
dims: 3
1 2 3

Hello
dims: 2 3
1 2 3
4 5 6


ones(2,3) * ones(3,6)
dims: 2 6
3 3 3 3 3 3
3 3 3 3 3 3


ones(15,3) * [0.5, 3.2, 0.4]
dims: 15
4.1 4.1 4.1 4.1 4.1 4.1 4.1 4.1 4.1 4.1 4.1 4.1 4.1 4.1 4.1

ones(4,2)
dims: 4 2
1 1
1 1
1 1
1 1
```


# Note
There are notes on how to engineer some features in this project.

## Putting object with non-trivial ctor to union
Why use union?

A: My personal laptop is in a quite old version of MacOS, and it doesn't support generic data containers, such as std::any and std::variant. Also, using boost in such a micro project is not an appealing solution to me.

tagged union & placement new
```cpp
struct Mat {
    std::vector<float> data;
    std::vector<int> dim;
};

class Value {
    // tagged union can have ctor & dtor in ISO C++
    union VALUE_Data {
        float       fp;
        int         _int;
        bool        _bool;
        std::string _str;
        Mat         matrix;
        VALUE_Data() {}
        ~VALUE_Data() {}
    };

    VALUE_Data data;
    // VOID, MATRIX are enum value that is used for RTTI
    RT_Value() : type(VOID) {};
    ...

    // placement new & tagged union for storing object in union
    explicit RT_Value(Mat val) : type(MATRIX) { new (&data.matrix) Mat(std::move(val)); };

    // do not forget that we break the rule of five
    RT_Value(const RT_Value& val);  // new (&data.matrix) Mat(val)
    RT_Value(RT_Value&& val) noexcept;
    RT_Value &operator=(RT_Value val);
}
```


## RAII helper for indentation control
```cpp
// AST_Printer.cpp
#define INDENT_EACH_STEP 2

struct Indent {
    explicit Indent(int &level) : level(level) { level += INDENT_EACH_STEP; }
    std::string get_indent() { return std::string(level, ' '); }
    ~Indent() { level -= INDENT_EACH_STEP; }
    int &level;
};

// level increment 2 every time we initialize an Indent object, so the initial
// level should be -1 *  2, otherwise we cannot get printed result starting without
// indentations.
void AST_Printer::AST_Printer() : cur_indent(-INDENT_EACH_STEP)
{}

void AST_Printer::visit_if(If_AST &expr) {
    Indent ind(cur_indent);
    std::cout << ind.get_indent() << "[IF_STMT]\n";
    std::cout << ind.get_indent() << "if ";
    expr.cond->accept(*this);
    std::cout << ind.get_indent() << " is not 0\n";
    expr.if_block->accept(*this);
    if (expr.else_block) {
        std::cout << ind.get_indent() << "[ELSE]\n";
        expr.else_block->accept(*this);
    }
    // destructor is evoked when this function call is finished, and it will decrease the cur_indent by 2
}
```

## RAII switcher helper
RAII with a simple bool lock can be used to print desired result in a recursive function.

```cpp
// control the print switch by RAII
struct Switcher {
    explicit Switcher(bool &switcher) : switcher(switcher) {
        // Is this object turns on the switch?
        inner = !switcher;
        // turn on switch
        if (!switcher) switcher = true;
    }
    ~Switcher() { if (inner) switcher = false; }
    bool &switcher;
    bool inner;
};

void AST_Printer::visit_mat(Matrix_AST &expr) {
    Indent ind(cur_indent);

    if (!no_info)
        os << ind.get_indent() << "[MATRIX]\n";

    if (expr.dim.empty()) {
        os << ind.get_indent() << "Empty matrix\n";
        return ;
    }
    if (!no_info) {
        os << ind.get_indent() << "dims: ";
        for (int i = 0; i < expr.dim.size()-1; i++)
            os << expr.dim[i] << ",";
        os << expr.dim.back() << "\n";
        os << ind.get_indent() << "value:\n";
    }

    // control print by RAII
    // by adding an inner lock in Switcher class, we can control the
    // print result in a recursive function.

    // only the first initialization can turn the switch on after
    // it is not alive, the switch will be turned off.
    Switcher switcher(no_info);

    for (int i = 0; i < expr.dim[0]; i++) {
        if (auto inner = dynamic_cast<Float_point_AST*> (expr.values[i].get())) {
            if (i == 0) {
                os << ind.get_indent();
            }
            os << inner->val << " ";
        } else {
            Expression_AST &mat = *(expr.values[i]);
            visit_mat(dynamic_cast<Matrix_AST&> (mat));
        }
    }
    os << "\n";
}

```

## Variadic template for pushing data elegantly
The paradigm below can be easily implemented by using variadic template, but it does not look great for extra function (make_pair).
```cpp
push_back(make_pair(a,b),
          make_pair(c,d),
          make_pair(e,f))
```

To do it more elegantly, we can unpack arguments by two, and use one class Entries to store remaining chunks
```cpp
// as terminator
template <typename STR>
void push_back(STR&& func, Runtime::builtin_func_t func_ptr) {
    funcs.emplace_back(std::forward<STR>(func), func_ptr);
}

template <typename STR, typename... Entries>
void push_back(STR&& func, Runtime::builtin_func_t func_ptr, Entries && ...args) {
    funcs.emplace_back(std::forward<STR>(func), func_ptr);
    push_back(std::forward<Entries> (args)...);
}

// then
string func_name   = "println";
string func_name_2 = "func_info";
reg->push_back(func_name,         builtin_println,
               move(func_name_2), builtin_print_func_args,
               "info",            builtin_print_statue);
```
