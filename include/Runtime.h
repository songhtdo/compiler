//
// Created by Lin Sinan on 2020-12-19.
//

#ifndef COMPILER_RUNTIME_H
#define COMPILER_RUNTIME_H

#include <unordered_map>
#include <queue>
#include <string>
#include "helper.h"

struct Block_AST;
struct Expression_AST;

namespace runtime_ns {

    enum Value_Type : int { FP, INT, BOOL, VOID };

    class RT_Value {
    public:
        explicit RT_Value() : type(VOID) {};
        explicit RT_Value(Value_Type type, float val) : type(type) { data.fp = val;}
        explicit RT_Value(Value_Type type, bool val) : type(type) { data._bool = val;}
        explicit RT_Value(Value_Type type, int val) : type(type) { data._int = val;}

        bool to_bool();
        template <int _Value_Type> bool is_type();

        RT_Value operator+(RT_Value rhs);
        RT_Value operator-(RT_Value rhs);
        RT_Value operator*(RT_Value rhs);
        RT_Value operator/(RT_Value rhs);
        RT_Value operator%(RT_Value rhs);

        RT_Value operator>(RT_Value rhs);
        RT_Value operator<(RT_Value rhs);
        RT_Value operator==(RT_Value rhs);
        RT_Value operator>=(RT_Value rhs);
        RT_Value operator<=(RT_Value rhs);
        RT_Value operator^(RT_Value rhs);
    public:
        Value_Type                                type {};
        union { int _int; float fp; bool _bool; } data {};
    };

    struct RT_Function {
        std::vector<std::string>        params_name;
        std::vector<RT_Value>           params;
        std::unique_ptr<Block_AST>      block;
        std::unique_ptr<Expression_AST> ret;
    };

    class Context {
    public:
        bool         has_variable(const std::string&);
        RT_Value     get_variable(const std::string&);
        void         creat_variable(const std::string&, RT_Value);
        void         creat_variables(std::vector<std::string>, std::vector<RT_Value>);

        bool         has_function(const std::string& name);
        RT_Function* get_function(const std::string& name);
        void         creat_function(std::string name, RT_Function* f);
    public:
        // store variables & functions
        std::unordered_map<std::string, RT_Value>     vars;
        std::unordered_map<std::string, RT_Function*> funcs;
    };

    class Runtime {
    public:
        using buildin_func_t = RT_Value (*)(Runtime*, RT_Function*);

        RT_Value       get_variable(const std::string&);
        void           creat_variable(const std::string&, RT_Value);
        void           creat_variables(std::vector<std::string>, std::vector<RT_Value>);

        RT_Function*   get_function(const std::string& name);
        void           creat_function(std::string name, RT_Function* f);

        buildin_func_t get_builtin_function(const std::string&);

        void creat_context();
        void ruin_context();

        static std::unique_ptr<Runtime> make_runtime();
    public:
        std::vector<Context*> contexts;
        std::unordered_map<std::string, buildin_func_t> builtin_func;
    };
}

#endif //COMPILER_RUNTIME_H