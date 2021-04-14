//
//  parse.cpp
//  MSDscriptCommandLine
//
//  Created by Michael Langston on 3/29/21.
//
#include "parse.hpp"


//Helper Functions for Parsing Functions
void consume_character(std::istream &in, int expected){
    int character_To_Check = in.get();
    if(character_To_Check != expected)
        throw std::runtime_error("The character being consumed doesn't match the character you expected.");
}

void skip_whitespace(std::istream &in){
    while (true) {
        int c = in.peek();
        if(!isspace(c))
            break;
        consume_character(in, c);
    }
}

void parse_keyword(std::istream &to_Parse, std::string expected_keyword){
    for (char expected_char : expected_keyword) {
        char character_To_Check = to_Parse.peek();
        if(expected_char == character_To_Check)
            consume_character(to_Parse, expected_char);
        else
            throw std::runtime_error("Unexpected keyword found.");
    }
}

PTR(Expr) parse_num(std::istream &to_Parse){
    int to_Return = 0;
    bool is_Negative = false;
    
    if(to_Parse.peek() == '-'){
        is_Negative = true;
        consume_character(to_Parse, '-');
    }
    
    //errorchecking for chars between negative sign and number
    if (!(isdigit(to_Parse.peek()))) {
        throw std::runtime_error("parse_num: and unexpected charater has been detected between \"-\" and the number...");
    }
    
    while (true) {
        int input_Charater = to_Parse.peek();
        if(isdigit(input_Charater)){
            consume_character(to_Parse, input_Charater);
            to_Return = (unsigned)to_Return * 10 + (input_Charater - '0');
            
            //This is a safety check for integer overflow in a single number (-2147483648 isn't valid)
            if(to_Return < 0)
                throw std::runtime_error("This number cannot be represented by the backing type 'int' becasue it causes memory overflow...");
        }
        else
            break;
    }
    
    if(is_Negative)
        to_Return = -to_Return;
    
    return NEW(NumExpr)(to_Return);
}

PTR(Expr) parse_var(std::istream &to_Parse){
    std::string to_Return = "";
    
    while (true) {
        char input_Charater = to_Parse.peek();
        if(isalpha(input_Charater)){
            consume_character(to_Parse, input_Charater);
            to_Return += input_Charater;
        }
        else
            break;
    }
    return NEW(VarExpr)(to_Return);
}


PTR(Expr) parse_let(std::istream &to_Parse){
    parse_keyword(to_Parse, "let");
    skip_whitespace(to_Parse);
    std::string lhs_name = parse_var(to_Parse)->to_string();
    
    skip_whitespace(to_Parse);
    parse_keyword(to_Parse, "=");
    PTR(Expr) rhs = parse_expr(to_Parse);
    
    parse_keyword(to_Parse, "_in");
    skip_whitespace(to_Parse);
    PTR(Expr) body = parse_expr(to_Parse);
    
    return NEW(LetExpr)(lhs_name, rhs, body);
}



PTR(Expr) parse_if(std::istream &to_Parse){
    parse_keyword(to_Parse, "if");
    skip_whitespace(to_Parse);
    PTR(Expr) comparison = parse_expr(to_Parse);
    
    skip_whitespace(to_Parse);
    parse_keyword(to_Parse, "_then");
    PTR(Expr) if_true = parse_expr(to_Parse);
    
    parse_keyword(to_Parse, "_else");
    skip_whitespace(to_Parse);
    PTR(Expr) if_false = parse_expr(to_Parse);
    
    return NEW(IfExpr)(comparison, if_true, if_false);
}



PTR(Expr) parse_fun(std::istream &to_Parse){
    parse_keyword(to_Parse, "un");
    skip_whitespace(to_Parse);
    PTR(Expr) formal_arg = parse_expr(to_Parse);
    
    skip_whitespace(to_Parse);
    PTR(Expr) body = parse_expr(to_Parse);
    
    return NEW(FunExpr)(formal_arg->to_string(), body);
}


//Parsing Functions
PTR(Expr) parse_expr(std::istream & to_Parse){
    PTR(Expr) e = parse_comparg(to_Parse);
    
    skip_whitespace(to_Parse);
    
    int c = to_Parse.peek();
    if (c == '=') {
        consume_character(to_Parse, '=');
        c = to_Parse.peek();
        if (c == '=') {
            consume_character(to_Parse, '=');
            PTR(Expr) rhs = parse_expr(to_Parse);
            return NEW(EqExpr)(e, rhs);
        }
        throw std::runtime_error("Invalid Operand...");
    }
    return e;
}

PTR(Expr) parse_comparg(std::istream & to_Parse){
    PTR(Expr) e = parse_addend(to_Parse);
    
    skip_whitespace(to_Parse);
    
    int c = to_Parse.peek();
    if (c == '+') {
        consume_character(to_Parse, '+');
        PTR(Expr) rhs = parse_comparg(to_Parse);
        return NEW(AddExpr)(e, rhs);
    }
    else
        return e;
}

PTR(Expr) parse_addend(std::istream &to_Parse){
    PTR(Expr) e = parse_multicand(to_Parse);
    
    skip_whitespace(to_Parse);
    
    int c = to_Parse.peek();
    if (c == '*') {
        consume_character(to_Parse, '*');
        PTR(Expr) rhs = parse_addend(to_Parse);
        return NEW(MultExpr)(e, rhs);
    }
    else
        return e;
}

PTR(Expr) parse_multicand(std::istream &to_Parse){
    PTR(Expr) e = parse_inner(to_Parse);
    skip_whitespace(to_Parse);
    while (to_Parse.peek() == '(') {
        consume_character(to_Parse, '(');
        PTR(Expr) actual_arg = parse_expr(to_Parse);
        consume_character(to_Parse, ')');
        skip_whitespace(to_Parse);
        e = NEW(CallExpr)(e, actual_arg);
    }
    return e;
}


PTR(Expr) parse_inner(std::istream &to_Parse){
    skip_whitespace(to_Parse);
    
    int c = to_Parse.peek();
    if ((c == '-') || isdigit(c))
        return parse_num(to_Parse);
    else if (c == '('){
        consume_character(to_Parse, '(');
        PTR(Expr) e = parse_expr(to_Parse);
        skip_whitespace(to_Parse);
        c = to_Parse.get();
        if (c != ')')
            throw std::runtime_error("One or more closing parentheses are missing.");
        return e;
    }
    else if (isalpha(c)){
        return parse_var(to_Parse);
    }
    else if (c == '_'){
        consume_character(to_Parse, '_');
        c = to_Parse.peek();
        if (c == 'l'){
            return parse_let(to_Parse);
        }
        else if (c == 't'){
            parse_keyword(to_Parse, "true");
            return NEW(BoolExpr)(true);
        }
        else if (c == 'f'){
            consume_character(to_Parse, 'f');
            c = to_Parse.peek();
            if (c == 'a') {
                parse_keyword(to_Parse, "alse");
                return NEW(BoolExpr)(false);
            }
            else if(c == 'u'){
                return parse_fun(to_Parse);
            }
            else{
                consume_character(to_Parse, c);
                throw std::runtime_error("Invalid Input...");
            }
        }
        else{
            return parse_if(to_Parse);
        }
    }
    else{
        consume_character(to_Parse, c);
        throw std::runtime_error("Invalid Input...");
    }
}


