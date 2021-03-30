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

TEST_CASE("consume_character"){
    {
        std::stringstream testing ("j");
        consume_character(testing, 'j');
        CHECK(testing.get() == EOF);
    }
    
    {
        std::stringstream testing ("p");
        CHECK_THROWS_WITH(consume_character(testing, 'j'), "The character being consumed doesn't match the character you expected.");
    }
   
}

void skip_whitespace(std::istream &in){
    while (true) {
        int c = in.peek();
        if(!isspace(c))
            break;
        consume_character(in, c);
    }
}

TEST_CASE("skip_whitespace"){
    {
        std::stringstream testing ("       j");
        skip_whitespace(testing);
        CHECK(testing.get() == 'j');
    }
    
    {
        std::stringstream testing ("j        ");
        skip_whitespace(testing);
        CHECK(testing.get() == 'j');
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

TEST_CASE("parse_keyword"){
    {
        std::stringstream testing ("_in");
        parse_keyword(testing, "_in");
        CHECK(testing.get() == EOF);
    }
    
    {
        std::stringstream testing ("_n");
        CHECK_THROWS_WITH(parse_keyword(testing, "_in"), "Unexpected keyword found.");
    }
}

PTR(Expr) parse_num(std::istream &to_Parse){
    int to_Return = 0;
    bool is_Negative = false;
    
    if(to_Parse.peek() == '-'){
        is_Negative = true;
        consume_character(to_Parse, '-');
    }
    if (!(isdigit(to_Parse.peek()))) {
        throw std::runtime_error("parse_num: and unexpected charater has been detected between \"-\" and the number...");
    }
    
    while (true) {
        int input_Charater = to_Parse.peek();
        if(isdigit(input_Charater)){
            consume_character(to_Parse, input_Charater);
            to_Return = to_Return * 10 + (input_Charater - '0');
        }
        else
            break;
    }
    
    if(is_Negative)
        to_Return = -to_Return;
    
    return NEW(NumExpr)(to_Return);
}

TEST_CASE("parse_num"){
    {
        std::stringstream testing ("33");
        CHECK(parse_num(testing)->equals(NEW(NumExpr)(33)));
    }
    
    {
        std::stringstream testing ("-33");
        CHECK(parse_num(testing)->equals(NEW(NumExpr)(-33)));
    }
    
    {
        std::stringstream testing ("--33");
        CHECK_THROWS_WITH(parse_num(testing), "parse_num: and unexpected charater has been detected between \"-\" and the number...");
    }
    
    {
        std::stringstream testing ("-");
        CHECK_THROWS_WITH(parse_num(testing), "parse_num: and unexpected charater has been detected between \"-\" and the number...");
    }
    
    {
        std::stringstream testing ("g");
        CHECK_THROWS_WITH(parse_num(testing), "parse_num: and unexpected charater has been detected between \"-\" and the number...");
    }
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

TEST_CASE("parse_var"){
    {
        std::stringstream testing ("g");
        CHECK(parse_var(testing)->equals(NEW(VarExpr)("g")));
    }
    
    {
        std::stringstream testing ("guess0");
        CHECK(parse_var(testing)->equals(NEW(VarExpr)("guess")));
    }
    
    {
        std::stringstream testing ("gu0ess");
        CHECK(parse_var(testing)->equals(NEW(VarExpr)("gu")));
    }
    
    {
        std::stringstream testing ("0guess");
        CHECK(parse_var(testing)->equals(NEW(VarExpr)("")));
    }
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

TEST_CASE("parse_let"){
    //No spaces
    {
        //the _ has been consumed at this point
        std::stringstream testing ("letx=5_inx+3");
        CHECK(parse_let(testing)->equals(NEW(LetExpr)("x", NEW(NumExpr)(5), NEW(AddExpr)(NEW(VarExpr)("x"), NEW(NumExpr) (3)))));
    }
    //some spaces
    {
        std::stringstream testing ("let  x  =   5   _in  x   +  3");
        CHECK(parse_let(testing)->equals(NEW(LetExpr)("x", NEW(NumExpr)(5), NEW(AddExpr)(NEW(VarExpr)("x"), NEW(NumExpr) (3)))));
    }
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

TEST_CASE("parse_if"){
    //No spaces
    {
        //the _ has been consumed at this point
        std::stringstream testing ("ifx==5_then5_else6");
        CHECK(parse_if(testing)->equals(NEW(IfExpr)(NEW(EqExpr)(NEW(VarExpr)("x"), NEW(NumExpr)(5)), NEW(NumExpr)(5), NEW(NumExpr)(6))));
    }
    //some spaces
    {
        std::stringstream testing ("if  \n  x   \n   ==    \n   5  \n    _then \n    5   \n   _else  \n    6");
        CHECK(parse_if(testing)->equals(NEW(IfExpr)(NEW(EqExpr)(NEW(VarExpr)("x"), NEW(NumExpr)(5)), NEW(NumExpr)(5), NEW(NumExpr)(6))));
    }
}

PTR(Expr) parse_fun(std::istream &to_Parse){
    parse_keyword(to_Parse, "un");
    skip_whitespace(to_Parse);
    PTR(Expr) formal_arg = parse_expr(to_Parse);
    
    skip_whitespace(to_Parse);
    PTR(Expr) body = parse_expr(to_Parse);
    
    return NEW(FunExpr)(formal_arg->to_string(), body);
}

TEST_CASE("parse_fun"){
    
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

TEST_CASE("parse expr"){
    {
        std::stringstream rep_cout (" 3 == 4");
        CHECK(parse_expr(rep_cout)->equals(NEW(EqExpr)(NEW(NumExpr)(3), NEW(NumExpr)(4))));
    }
    {
        std::stringstream rep_cout (" 3 =? 4");
        CHECK_THROWS_WITH(parse_expr(rep_cout), "Invalid Operand...");
    }
    {
        std::stringstream rep_cout (" 3 + 4");
        CHECK(parse_expr(rep_cout)->equals(NEW(AddExpr)(NEW(NumExpr)(3), NEW(NumExpr)(4))));
    }
    {
        std::stringstream rep_cout (" 3 * 4");
        CHECK(parse_expr(rep_cout)->equals(NEW(MultExpr)(NEW(NumExpr)(3), NEW(NumExpr)(4))));
    }
    {
        std::stringstream rep_cout (" 3 ");
        CHECK(parse_expr(rep_cout)->equals(NEW(NumExpr)(3)));
    }
    {
        std::stringstream rep_cout ("    _let     f     =    _fun        (x)   x+    1     _in   f    (   5   )   ");
        CHECK(parse_expr(rep_cout)->equals(NEW(LetExpr)("f", NEW(FunExpr)("x",NEW(AddExpr)(NEW(VarExpr)("x"), NEW(NumExpr)(1))), NEW(CallExpr)(NEW(VarExpr)("f"), NEW(NumExpr)(5)))));
    }
    {
        std::stringstream rep_cout ("_let factrl = _fun (factrl) _fun (x) _if x == 1 _then 1 _else x * factrl(factrl)(x + -1) _in  factrl(factrl)(10)");
        PTR(EqExpr) conditional = NEW(EqExpr)(NEW(VarExpr)("x"), NEW(NumExpr)(1));
        PTR(MultExpr) multElse = NEW(MultExpr)(NEW(VarExpr)("x"), NEW(CallExpr)(NEW(CallExpr)(NEW(VarExpr)("factrl"), NEW(VarExpr)("factrl")), NEW(AddExpr)(NEW(VarExpr)("x"), NEW(NumExpr)(-1))));
        PTR(IfExpr) insideIfElse = NEW(IfExpr)(conditional, NEW(NumExpr)(1), multElse);
        PTR(FunExpr) insidefunction = NEW(FunExpr)("x", insideIfElse);
        PTR(FunExpr) function_rhs = NEW(FunExpr)("factrl", insidefunction);
        PTR(LetExpr) overAll = NEW(LetExpr)("factrl", function_rhs, NEW(CallExpr)(NEW(CallExpr)(NEW(VarExpr)("factrl"), NEW(VarExpr)("factrl")), NEW(NumExpr)(10)));
        CHECK(parse_expr(rep_cout)->equals(overAll));
    }
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

TEST_CASE("parse_comparg"){
    {
        std::stringstream rep_cout (" 3 + 4");
        CHECK(parse_comparg(rep_cout)->equals(NEW(AddExpr)(NEW(NumExpr)(3), NEW(NumExpr)(4))));
    }
    {
        std::stringstream rep_cout (" 3 * 4");
        CHECK(parse_comparg(rep_cout)->equals(NEW(MultExpr)(NEW(NumExpr)(3), NEW(NumExpr)(4))));
    }
    {
        std::stringstream rep_cout (" 3 ");
        CHECK(parse_comparg(rep_cout)->equals(NEW(NumExpr)(3)));
    }
    {
        //the == is ignored and just 3 is returned according the present lodgic we have
        std::stringstream rep_cout (" 3 == 4");
        CHECK(parse_comparg(rep_cout)->equals(NEW(NumExpr)(3)));
    }
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

TEST_CASE("parse addend"){
    {
        std::stringstream rep_cout (" 3 * 4");
        CHECK(parse_addend(rep_cout)->equals(NEW(MultExpr)(NEW(NumExpr)(3), NEW(NumExpr)(4))));
    }
    {
        std::stringstream rep_cout (" 3 + 4");
        CHECK(parse_addend(rep_cout)->equals(NEW(NumExpr)(3)));
    }
    {
        std::stringstream rep_cout (" 3 ");
        CHECK(parse_addend(rep_cout)->equals(NEW(NumExpr)(3)));
    }
    {
        //the == is ignored and just 3 is returned according the lodgic
        std::stringstream rep_cout (" 3 == 4");
        CHECK(parse_addend(rep_cout)->equals(NEW(NumExpr)(3)));
    }
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

TEST_CASE("parse multicand"){
    {
        std::stringstream rep_cout ("         factrl   (   factrl   )       (   10   )   ");
        CHECK(parse_multicand(rep_cout)->equals(NEW(CallExpr)(NEW(CallExpr)(NEW(VarExpr)("factrl"), NEW(VarExpr)("factrl")), NEW(NumExpr)(10))));
    }
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

TEST_CASE("parse inner"){
    {
        std::stringstream testing ("@");
        CHECK_THROWS_WITH(parse_inner(testing), "Invalid Input...");
    }
    {
        std::stringstream testing ("_@");
        CHECK_THROWS_WITH(parse_inner(testing), "Unexpected keyword found.");
    }
    {
        std::stringstream testing ("_f@");
        CHECK_THROWS_WITH(parse_inner(testing), "Invalid Input...");
    }
    {
        std::stringstream testing ("(4");
        CHECK_THROWS_WITH(parse_inner(testing), "One or more closing parentheses are missing.");
    }
    {
        std::stringstream testing ("(4)");
        CHECK(parse_inner(testing)->equals(NEW(NumExpr)(4)));
    }
    {
        std::stringstream testing ("_ifx==5_then5_else6");
        CHECK(parse_inner(testing)->equals(NEW(IfExpr)(NEW(EqExpr)(NEW(VarExpr)("x"), NEW(NumExpr)(5)), NEW(NumExpr)(5), NEW(NumExpr)(6))));
    }
    {
        //the _ has been consumed at this point
        std::stringstream testing ("_letx=5_inx+3");
        CHECK(parse_inner(testing)->equals(NEW(LetExpr)("x", NEW(NumExpr)(5), NEW(AddExpr)(NEW(VarExpr)("x"), NEW(NumExpr) (3)))));
    }
    {
        std::stringstream testing ("_true");
        CHECK(parse_inner(testing)->equals(NEW(BoolExpr)(true)));
    }
    {
        std::stringstream testing ("_false");
        CHECK(parse_inner(testing)->equals(NEW(BoolExpr)(false)));
    }
}
