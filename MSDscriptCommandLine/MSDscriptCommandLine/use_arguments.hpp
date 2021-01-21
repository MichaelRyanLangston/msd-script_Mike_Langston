//
//  use_arguments.hpp
//  MSDscriptCommandLine
//
//  Created by Michael Langston on 1/19/21.
//

#ifndef use_arguments_hpp
#define use_arguments_hpp

#include <stdio.h>
#include <iostream>
#include <string>

/*
 This function takes commands:
 --help => Displays this message.
 --test => Prints test to the screen. Can only be utilized once.
 Any other commands will result in an error.
 */
void use_arguments(const int arraySize, const char* array[]);

#endif /* use_arguments_hpp */
