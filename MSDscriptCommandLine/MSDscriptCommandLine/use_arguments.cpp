//
//  use_arguments.cpp
//  MSDscriptCommandLine
//
//  Created by Michael Langston on 1/19/21.
//

#define CATCH_CONFIG_RUNNER
#include "catch.h"
#include "use_arguments.hpp"

/*
 This function takes commands:
 --help => Displays this message.
 --test => Prints test to the screen. Can only be utilized once.
 Any other commands will result in an error.
 */
void use_arguments(const int arraySize, const char* array[]){
    if(arraySize == 1){
        std::cout << "No arguments found.\n";
        exit(2);
    }
    bool testSeen = false;
    for (int i = 1; i < arraySize; i++){
        if ((std::string)array[i] == "--help"){
            std::cout << "This function takes commands:\n";
            std::cout << "--help => Displays this message.\n";
            std::cout << "--test => Prints Test Passed to the screen. Can only be utilized once.\n";
            std::cout << "Any other commands will result in an error.\n";
            exit(0);
        }
        else if ((std::string)array[i] == "--test" && !testSeen){
            testSeen = true;
            if(Catch::Session().run(1, array) != 0)
                exit(1);
            continue;
        }
        std::cerr << "Invalid command\n";
        exit(1);
    }
}
