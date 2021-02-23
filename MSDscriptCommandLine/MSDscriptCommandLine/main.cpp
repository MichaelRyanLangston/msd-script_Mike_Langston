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
    
    try {
        use_arguments(argc, argv);
        return 0;
    } catch (std::runtime_error exn) {
        std::cerr << exn.what() << "\n";
        return 1;
    }
}
