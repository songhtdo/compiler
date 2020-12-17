//
// Created by Lin Sinan on 2020-12-16.
//

//
// Created by Lin Sinan on 2020-12-13.
//

#include "Parser.h"

static int TEST_COUNT = 1;
#define TEST_NAME(X) printf("==============TEST %d: %-10s==============\n", TEST_COUNT++, X);

void print_expression(Expression_AST *exp);

//const char *code =
//        "var x = 12 + x * 3\n"
//        "var y = x + 1";

//const char *code =
//        "var x = ((12 + x) * 3) / ((2 - 44) / -4)\n";

const char *code =
        "func pow(x, y) {\n"
        "   return x^y\n"
        "}\n"
        "# test !\n"
        "var x = ((12+4) * 8) \n"
        "if (1+2 >= 3) {\n\n"
        "   # test !\n"
        "   var z = 12 + x * 3\n"
        "   x = z + 26 % -21\n"
        "} else {"
        "   x = (12 ^ 2) * pow(2,3) \n"
        "   x = fun(x) \n"
        "}";

void driver(std::unique_ptr<Parser> &parser) {
    std::vector<std::unique_ptr<Expression_AST>> v = parser->parse();
    if (v.empty()) return ;

    int line = 1;
    for (auto &&expr : v) {
        printf("=========line %d=========\n", line++);
        expr->print();
    }
}

int main() {
    std::unique_ptr<Parser> parser = Parser::make_parser(code);

    TEST_NAME("simple def")
    driver(parser);
}