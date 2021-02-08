//
//  main.cpp
//  MSDscriptCommandLine
//
//  Created by Michael Langston on 1/19/21.
//


#include "use_arguments.hpp"
#include "expr.hpp"
#include <sstream>
int main(int argc, const char* argv[]){
//    use_arguments(argc, argv);
    _let* print_test = new _let("x", new Num(5), new Add(new _let("y", new Num(3), new Add(new Var("y"), new Num(2))), new Var("x")));
    std::stringstream testing("");
    print_test->pretty_print(testing);
    std::cout << testing.str();
    return 0;
}
