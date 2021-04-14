//
//  parse.hpp
//  MSDscriptCommandLine
//
//  Created by Michael Langston on 3/29/21.
//

#ifndef parse_hpp
#define parse_hpp

#include <stdio.h>
#include <iostream>
#include <string>
#include <sstream>
#include <stdexcept>
#include "pointermgmt.h"
#include "expr.hpp"

//Parsing Helper Functions
void consume_character(std::istream &in, int expected);
void skip_whitespace(std::istream &in);
void parse_keyword(std::istream &to_Parse, std::string expected_keyword);
PTR(Expr) parse_num(std::istream &to_Parse);
PTR(Expr) parse_var(std::istream &to_Parse);
PTR(Expr) parse_let(std::istream &to_Parse);
PTR(Expr) parse_if(std::istream &to_Parse);
PTR(Expr) parse_fun(std::istream &to_Parse);

//Parsing Functions
PTR(Expr) parse_expr(std::istream & to_Parse);
PTR(Expr) parse_comparg(std::istream & to_Parse);
PTR(Expr) parse_addend(std::istream &to_Parse);
PTR(Expr) parse_multicand(std::istream &to_Parse);
PTR(Expr) parse_inner(std::istream &to_Parse);

#endif /* parse_hpp */
