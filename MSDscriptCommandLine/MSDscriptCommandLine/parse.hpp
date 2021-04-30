//
//  parse.hpp
//  MSDscriptCommandLine
//
//  Created by Michael Langston on 3/29/21.
//

#ifndef parse_hpp
#define parse_hpp

#include <stdio.h>
#include <sstream>
#include "catch.h"
#include "pointermgmt.h"
#include "parse.hpp"
#include "expr.hpp"
#include "val.hpp"
#include "env.hpp"
#include "step.hpp"
#include "continue.hpp"

/* Parsing Helper Functions */

// A function used to check and remvoe a charater from an input stream
void consume_character(std::istream &in, int expected);

//A function used to remove tab or space charaters from an input stream
void skip_whitespace(std::istream &in);

//A function used to parse keywords from an input stream
void parse_keyword(std::istream &to_Parse, std::string expected_keyword);

/* Specfic Parsing Functions */

//Translates a number charater into number expression
PTR(Expr) parse_num(std::istream &to_Parse);

//Translates a variable charater or charaters into a variable expression
PTR(Expr) parse_var(std::istream &to_Parse);

//Translates let keywords into a let expression
PTR(Expr) parse_let(std::istream &to_Parse);

//Translates if keywords into a if expression
PTR(Expr) parse_if(std::istream &to_Parse);

//Translates function keywords into a functions expression
PTR(Expr) parse_fun(std::istream &to_Parse);

/* Parsing Functions */

//Begining of the parser. Translates strings into EqExprs Calls parse_comparg.
PTR(Expr) parse_expr(std::istream & to_Parse);

//Translates strings into a AddExpr. Calls parse_addend.
PTR(Expr) parse_comparg(std::istream & to_Parse);

//Translates strings into a MultExpr. Calls parse_ multicand.
PTR(Expr) parse_addend(std::istream &to_Parse);

//Translates strings into a CallExpr. Calls parse_inner.
PTR(Expr) parse_multicand(std::istream &to_Parse);

//Translates strings by parising and calling approprite Specfic Parsing Functions. Also handles some basic errors in the input string
PTR(Expr) parse_inner(std::istream &to_Parse);

#endif /* parse_hpp */
