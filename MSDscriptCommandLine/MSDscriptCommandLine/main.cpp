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
    std::cout << "_let x = 5\n_in  (_let y = 3\n      _in  y + 2) + x" << std::endl;
    return 0;
}
