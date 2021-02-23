//
//  main.cpp
//  test_msdscript
//
//  Created by Michael Langston on 2/21/21.
//

#include <iostream>
#include <string>
#include "exec.h"


//prototyped functions
std::string generate_input();
std::string generate_alpha_string();
std::string generate_variable();


int main(int argc, const char * argv[]) {
    if(argc == 2){
        srand((unsigned int)clock());
        const char* test_print[2] = {argv[1],"--print"};
        const char* test_interp[2] = {argv[1], "--interp"};
        for (int i = 0; i < 100; i++){
            std::string input_String = generate_input();
            std::cout <<"Input: " << input_String << std::endl;
            
            ExecResult interp_Results = exec_program(2, test_interp, input_String);
            ExecResult print_Results = exec_program(2, test_print, input_String);
            ExecResult feedback_Interp_Results = exec_program(2, test_interp, print_Results.out);
            ExecResult feedback_Print_Results = exec_program(2, test_print, print_Results.out);
            
            if(feedback_Interp_Results.out.compare(interp_Results.out) != 0 || feedback_Interp_Results.err.compare(interp_Results.err) != 0 || feedback_Interp_Results.exit_code != interp_Results.exit_code){
                std::cout << "Interp out: " << interp_Results.out << std::endl;
                std::cout << "Interp error: " << interp_Results.err;
                std::cout << "Interp exit code: " << interp_Results.exit_code << std::endl;
                std::cout << "Feedback out: " << feedback_Interp_Results.out << std::endl;
                std::cout << "Feedback error: " << feedback_Interp_Results.err;
                std::cout << "Feedback exit code: " << feedback_Interp_Results.exit_code << std::endl;
                throw std::runtime_error("Mismatched items. TEST FAIL");
            }
            
            if(feedback_Print_Results.out.compare(print_Results.out) != 0 || feedback_Print_Results.err.compare(print_Results.err) != 0 || feedback_Print_Results.exit_code != print_Results.exit_code){
                std::cout << "Print out: " << print_Results.out << std::endl;
                std::cout << "Print error: " << print_Results.err;
                std::cout << "Print exit code: " << print_Results.exit_code << std::endl;
                std::cout << "Feedback out: " << feedback_Print_Results.out << std::endl;
                std::cout << "Feedback error: " << feedback_Print_Results.err;
                std::cout << "Feedback exit code: " << feedback_Print_Results.exit_code << std::endl;
                throw std::runtime_error("Mismatched items. TEST FAIL");
            }
        }
    }
    else if(argc == 3){
        srand((unsigned int)clock());
        const char* test_Program1_print[2] = {argv[1],"--print"};
        const char* test_Program1_interp[2] = {argv[1], "--interp"};
        const char* test_Program2_print[2] = {argv[2],"--print"};
        const char* test_Program2_interp[2] = {argv[2], "--interp"};
        for(int i = 0; i < 100; i++){
            std::string input_String = generate_input();
            std::cout <<"Input: " << input_String << std::endl;
            
            ExecResult program1_Print_Results = exec_program(2, test_Program1_print, input_String);
            ExecResult program1_Interp_Results = exec_program(2, test_Program1_interp, input_String);
            ExecResult program2_Print_Results = exec_program(2, test_Program2_print, input_String);
            ExecResult program2_Interp_Results = exec_program(2, test_Program2_interp, input_String);
            
            if (program1_Interp_Results.out != program2_Interp_Results.out) {
                std::cout <<"Interp Test Failed\n";
                std::cout << "Program 1: " << program1_Interp_Results.out << " " << program1_Interp_Results.err << " "  << program1_Interp_Results.exit_code << std::endl;
                std::cout << "Program 2: " << program2_Interp_Results.out << " "  << program2_Interp_Results.err << " "  << program2_Interp_Results.exit_code << std::endl;
                throw std::runtime_error("Mismatched items. TEST FAIL");
            }
            
            if (program1_Print_Results.out != program2_Print_Results.out) {
                std::cout <<"Print Test Failed\n";
                std::cout << "Program 1: " << program1_Print_Results.out << " " << program1_Print_Results.err << " "  << program1_Print_Results.exit_code << std::endl;
                std::cout << "Program 2: " << program2_Print_Results.out << " "  << program2_Print_Results.err << " "  << program2_Print_Results.exit_code << std::endl;
                throw std::runtime_error("Mismatched items. TEST FAIL");
            }
        }
    }
    else{
        throw std::runtime_error("Invalid arguments passed...");
    }
    return 0;
}


std::string generate_input(){
    if ((rand() % 100) < 55) {
        int pick = rand() % 100;
        if (pick < 75)
            return std::to_string(rand() % 999);
        else {
            return generate_variable();
        }
    }
    else if ((rand() % 100) < 20){
        return " ( " + generate_input() + " ) ";
    }
    else if ((rand() % 100) < 80){
        int mult_or_add = rand() % 2;
        if (mult_or_add == 0)
            return generate_input() + " + " + generate_input();
        else
            return generate_input() + " * " + generate_input();
    }
    else
        return " _let " + generate_variable() + " = " + generate_input() + " _in " + generate_input();
}

std::string generate_variable(){
    int var_Type = rand() % 100;
    if (var_Type < 90) {
        return "x";
    }
    return generate_alpha_string();
}

std::string generate_alpha_string(){
    std::string toReturn = "";
    int stringLength;
    while (true) {
        if((stringLength = rand() % 10) != 0){
            for (int i = 0; i < stringLength; i++) {
                char randomAlpha;
                while (!isalpha((randomAlpha = rand() % 123)));
                toReturn += randomAlpha;
            }
            break;
        }
    }
    return toReturn;
}
