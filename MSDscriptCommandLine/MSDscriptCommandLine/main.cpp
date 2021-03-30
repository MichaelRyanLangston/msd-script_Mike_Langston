//
//  main.cpp
//  MSDscriptCommandLine
//
//  Created by Michael Langston on 1/19/21.
//

#define CATCH_CONFIG_RUNNER
#include "catch.h"
#include "expr.hpp"
#include "val.hpp"
#include "parse.hpp"
#include "env.hpp"
#include <sstream>


void use_arguments(const int arraySize, const char* array[]){
    if(arraySize == 1){
        std::cout << "No arguments found.\n";
        exit(2);
    }
    if ((std::string)array[1] == "--help"){
        std::cout << "This program takes the following commands:\n";
        std::cout << "--help => Displays this message.\n";
        std::cout << "--test => Runs all tests in the program\n";
        std::cout << "--interp => Takes the given argument, interprets it, and exits with 0 if successful.\n";
        std::cout << "--print => Takes the given argument, prints it, and exits with 0 if successful.\n";
        std::cout << "Any additional flags are ignored.\n";
        std::cout << "Arguments that are passed and not specified above result in an \"Invalid command\" error.\n";
        exit(0);
    }
    else if ((std::string)array[1] == "--test"){
        if(Catch::Session().run(1, array) != 0)
            exit(1);
        exit(0);
    }
    else if ((std::string)array[1] == "--interp"){
        while (true) {
            PTR(Expr) e = parse_expr(std::cin);
            std::cout << e->interp(EmptyEnv::empty)->make_string() + "\n";
            skip_whitespace(std::cin);
            if (std::cin.eof()) {
                break;
            }
        }
        exit(0);
    }
    else if ((std::string)array[1] == "--print"){
        while (true) {
            PTR(Expr) e = parse_expr(std::cin);
            e->print(std::cout);
            std::cout << std::endl;
            skip_whitespace(std::cin);
            if (std::cin.eof()) {
                break;
            }
        }
        exit(0);
    }
    else if((std::string)array[1] == "--pretty_print"){
        throw std::runtime_error("--pretty_print is not supported in this msdscript implementation.");
    }
    std::cerr << "Invalid command\n";
    exit(1);
}


int main(int argc, const char* argv[]){
    try {
        use_arguments(argc, argv);
        return 0;
    } catch (std::runtime_error exn) {
        std::cerr << exn.what() << "\n";
        return 1;
    }
}
