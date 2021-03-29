//
//  expr.cpp
//  MSDscriptCommandLine
//
//  Created by Michael Langston on 1/24/21.
//

#include "expr.hpp"
#include "val.hpp"
#include "catch.h"
#include <stdexcept>
#include <sstream>
#include <iostream>



/*Exper Interface Implemntations*/
//Methods
std::string Expr::to_string(){
    std::stringstream stringify("");
    this->print(stringify);
    return stringify.str();
}

/* Depricated */
//void Expr::pretty_print(std::ostream& out){
//    pretty_print_at(out, print_group_none, 0);
//}

//Method Tests
TEST_CASE("Exper"){
    CHECK((NEW(MultExpr)(NEW(AddExpr)(NEW(NumExpr)(0), NEW(VarExpr)("y")), NEW(NumExpr)(1)))->to_string() == "((0+y)*1)");
}

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





/*Num Implementation*/
//Default Constructor
NumExpr::NumExpr(int rep){
    this->rep = rep;
}

//Methods
bool NumExpr::equals(PTR(Expr)e){
    PTR(NumExpr)ne = CAST(NumExpr)(e);
    if (ne == NULL)
        return false;
    else
        return this->rep == ne->rep;
}

PTR(Val) NumExpr::interp(){
    return NEW(NumVal)(this->rep);
}

bool NumExpr::has_variable(){
    return false;
}

PTR(Expr) NumExpr::subst(std::string s, PTR(Expr)e){
    return THIS;
}

void NumExpr::print(std::ostream& out){
    out << this->rep;
}

/*Depricated*/
//void Num::pretty_print_at(std::ostream& out, print_mode_t mode, int num){
//    print(out);
//}

//Method Tests
TEST_CASE("Num"){
    
    //Test Variables
        PTR(NumExpr) one = NEW(NumExpr)(1);
        PTR(NumExpr) two = NEW(NumExpr)(2);
        PTR(MultExpr) mult_one_two = NEW(MultExpr)(one,two);
        PTR(AddExpr) add_one_two = NEW(AddExpr)(one,two);

    //Checking Num Equality
        CHECK(one->equals(NEW(NumExpr)(1)));

    //Checking Num Inequality
        CHECK(!(one->equals(two)));

    //Checking Class Inequality
        CHECK(!(one->equals(mult_one_two)));
        CHECK(!(one->equals(add_one_two)));

    //Checking interp()
        CHECK(one->interp()->equals(NEW(NumVal)(1)));
        CHECK(two->interp()->equals(NEW(NumVal)(2)));

    //Checking has_variable()
        CHECK(one->has_variable() == false);
        CHECK(two->has_variable() == false);

    //Checking subst()
        CHECK(one->subst("s", two)->equals(NEW(NumExpr)(1)));
        CHECK(two->subst("s", one)->equals(NEW(NumExpr)(2)));
    
    //Checking print()
        {
            std::stringstream rep_cout ("");
            one->print(rep_cout);
            CHECK(rep_cout.str() == "1");
        }
    /*Depricated*/
    //Checking pretty_print()
//        {
//            std::stringstream rep_cout ("");
//            one->pretty_print(rep_cout);
//            CHECK(rep_cout.str() == "1");
//        }
}






/*Add implementations*/
//Default Constructor
AddExpr::AddExpr(PTR(Expr)lhs, PTR(Expr)rhs) {
    this->lhs = lhs;
    this->rhs = rhs;
}

//Methods
bool AddExpr::equals(PTR(Expr)e){
    PTR(AddExpr)ae = CAST(AddExpr)(e);
    if (ae == NULL)
        return false;
    else
        return (this->lhs->equals(ae->lhs) && this->rhs->equals(ae->rhs));
}

PTR(Val) AddExpr::interp(){
    return this->lhs->interp()->add_to(this->rhs->interp());
}

bool AddExpr::has_variable(){
    return this->lhs->has_variable() || this->rhs->has_variable();
}

PTR(Expr) AddExpr::subst(std::string s, PTR(Expr)e){
    return NEW(AddExpr)(this->lhs->subst(s, e), this->rhs->subst(s, e));
}

void AddExpr::print(std::ostream& out){
    out << "(";
    this->lhs->print(out);
    out << "+";
    this->rhs->print(out);
    out << ")";
}


/*Depricated*/
//void Add::pretty_print_at(std::ostream& out, print_mode_t mode, int num){
//    if(mode == print_group_none){
//        this->lhs->pretty_print_at(out, print_group_add, 0);
//        out << " + ";
//        this->rhs->pretty_print_at(out, print_group_none, 0);
//    }
//    else {
//        out << "(";
//        this->lhs->pretty_print_at(out, print_group_add, 0);
//        out << " + ";
//        this->rhs->pretty_print_at(out, print_group_none, 0);
//        out << ")";
//    }
//}

//Method Tests
TEST_CASE("Add"){
    
    //Test Varialbels
        PTR(NumExpr) one = NEW(NumExpr)(1);
        PTR(NumExpr) two = NEW(NumExpr)(2);
        
        PTR(AddExpr) one_two = NEW(AddExpr)(one,two);
        PTR(AddExpr) two_one = NEW(AddExpr)(two,one);
        PTR(AddExpr) add_two_add_exper = NEW(AddExpr)(one_two, two_one);
        PTR(AddExpr) vars = NEW(AddExpr)(NEW(VarExpr)("x"), NEW(NumExpr)(3));
        PTR(AddExpr) vars2 = NEW(AddExpr)(NEW(NumExpr)(8), NEW(VarExpr)("y"));
        PTR(AddExpr) vars3 = NEW(AddExpr)(one_two, NEW(AddExpr)(NEW(VarExpr)("z"), NEW(NumExpr)(7)));
        PTR(AddExpr) vars4 = NEW(AddExpr)(NEW(AddExpr)(NEW(VarExpr)("a"), NEW(NumExpr)(3)), two_one);
        
        
        PTR(MultExpr) mult_one_two = NEW(MultExpr)(one,two);
        PTR(MultExpr) mult_two_one = NEW(MultExpr)(two,one);
        PTR(AddExpr) add_two_mult_exper = NEW(AddExpr)(mult_one_two, mult_two_one);
    
    
    //Checking Add Equality
        CHECK(one_two->equals(NEW(AddExpr)(NEW(NumExpr)(1), NEW(NumExpr)(2))));
    
    //Checking Add Inequality
        CHECK(!(one_two->equals(two_one)));
    
    //Checking Class Inequality;
        CHECK(!(one_two->equals(one)));
        CHECK(!(one_two->equals(mult_one_two)));
    
    //Checking interp()
        CHECK(one_two->interp()->equals(NEW(NumVal)(3)));
        CHECK(add_two_add_exper->interp()->equals(NEW(NumVal)(6)));
        CHECK(add_two_mult_exper->interp()->equals(NEW(NumVal)(4)));
        CHECK_THROWS_WITH(vars->interp(), "The variable has no value assigned to it yet.");
    
    //Checking has_variable()
        CHECK(vars->has_variable() == true);
        CHECK(vars2->has_variable() == true);
        CHECK(vars3->has_variable() == true);
        CHECK(vars4->has_variable() == true);
        CHECK(add_two_add_exper->has_variable() == false);
        CHECK(add_two_mult_exper->has_variable() == false);
    
    //Checking subst()
        //substitution with each avaiable Exper object.
            CHECK((NEW(AddExpr)(NEW(VarExpr)("x"), NEW(NumExpr)(7)))->subst("x", NEW(VarExpr)("y"))->equals(NEW(AddExpr)(NEW(VarExpr)("y"), NEW(NumExpr)(7))));
            CHECK((NEW(AddExpr)(NEW(VarExpr)("x"), NEW(NumExpr)(7)))->subst("x", NEW(NumExpr)(3))->equals(NEW(AddExpr)(NEW(NumExpr)(3), NEW(NumExpr)(7))));
            CHECK((NEW(AddExpr)(NEW(VarExpr)("x"), NEW(NumExpr)(7)))->subst("x", NEW(AddExpr)(NEW(VarExpr)("x"), NEW(NumExpr)(8)))->equals(NEW(AddExpr)(NEW(AddExpr)(NEW(VarExpr)("x"), NEW(NumExpr)(8)), NEW(NumExpr)(7))));
            CHECK((NEW(AddExpr)(NEW(VarExpr)("x"), NEW(NumExpr)(7)))->subst("x", NEW(MultExpr)(NEW(VarExpr)("x"), NEW(NumExpr)(8)))->equals(NEW(AddExpr)(NEW(MultExpr)(NEW(VarExpr)("x"), NEW(NumExpr)(8)), NEW(NumExpr)(7))));
        //nested substitution
            CHECK((NEW(AddExpr)(NEW(NumExpr)(3), NEW(AddExpr)(NEW(NumExpr)(6), NEW(VarExpr)("y"))))->subst("y", NEW(VarExpr)("x"))->equals(NEW(AddExpr)(NEW(NumExpr)(3), NEW(AddExpr)(NEW(NumExpr)(6), NEW(VarExpr)("x")))));
        //no Var object present
            CHECK((NEW(AddExpr)(NEW(NumExpr)(8), NEW(NumExpr)(7)))->subst("x", NEW(VarExpr)("y"))->equals(NEW(AddExpr)(NEW(NumExpr)(8), NEW(NumExpr)(7))));
        //no matching string
            CHECK((NEW(AddExpr)(NEW(VarExpr)("x"), NEW(NumExpr)(7)))->subst("mismatch", NEW(VarExpr)("y"))->equals(NEW(AddExpr)(NEW(VarExpr)("x"), NEW(NumExpr)(7))));
    
    //Checking print()
        //No nesting
            {
                std::stringstream rep_cout ("");
                (NEW(AddExpr)(NEW(NumExpr)(1), NEW(VarExpr)("x")))->print(rep_cout);
                CHECK(rep_cout.str() == "(1+x)");
            }
        //Nested right
            {
                std::stringstream rep_cout("");
                (NEW(AddExpr)(NEW(NumExpr)(3),NEW(AddExpr)(NEW(NumExpr)(1), NEW(VarExpr)("x"))))->print(rep_cout);
                CHECK(rep_cout.str() == "(3+(1+x))");
            }
        //Nested left
            {
                std::stringstream rep_cout("");
                (NEW(AddExpr)(NEW(AddExpr)(NEW(NumExpr)(1), NEW(VarExpr)("x")), NEW(NumExpr)(3)))->print(rep_cout);
                CHECK(rep_cout.str() == "((1+x)+3)");
            }
    /*Depricated*/
//    //Checking pretty_print()
//        //No nesting
//            {
//                std::stringstream rep_cout ("");
//                (new Add(new Num(1), new Var("x")))->pretty_print(rep_cout);
//                CHECK(rep_cout.str() == "1 + x");
//            }
//        //Add w/ Nested right Add
//            {
//                std::stringstream rep_cout("");
//                (new Add(new Num(3),new Add(new Num(1), new Var("x"))))->pretty_print(rep_cout);
//                CHECK(rep_cout.str() == "3 + 1 + x");
//            }
//        //Add w/ Nested left Add
//            {
//                std::stringstream rep_cout("");
//                (new Add(new Add(new Num(1), new Var("x")), new Num(3)))->pretty_print(rep_cout);
//                CHECK(rep_cout.str() == "(1 + x) + 3");
//            }
//        //Add w/ Nested right Mult
//            {
//                std::stringstream rep_cout("");
//                (new Add(new Num(3), new Mult(new Num(1), new Var("x"))))->pretty_print(rep_cout);
//                CHECK(rep_cout.str() == "3 + 1 * x");
//            }
//        //Add w/ Nested left Mult
//            {
//                std::stringstream rep_cout("");
//                (new Add(new Mult(new Num(1), new Var("x")), new Num(3)))->pretty_print(rep_cout);
//                CHECK(rep_cout.str() == "1 * x + 3");
//            }
}






/*Mult implementations*/
//Default Constructor
MultExpr::MultExpr(PTR(Expr)lhs, PTR(Expr)rhs) {
    this->lhs = lhs;
    this->rhs = rhs;
}

//Methods
bool MultExpr::equals(PTR(Expr)e){
    PTR(MultExpr) me = CAST(MultExpr)(e);
    if (me == NULL)
        return false;
    else
        return (this->lhs->equals(me->lhs) && this->rhs->equals(me->rhs));
}

PTR(Val) MultExpr::interp(){
    return this->lhs->interp()->mult_by(this->rhs->interp());
}

bool MultExpr::has_variable(){
    return this->lhs->has_variable() || this->rhs->has_variable();
}

PTR(Expr) MultExpr::subst(std::string s, PTR(Expr)e){
    return NEW(MultExpr)(this->lhs->subst(s, e), this->rhs->subst(s, e));
}

void MultExpr::print(std::ostream& out){
    out << "(";
    this->lhs->print(out);
    out << "*";
    this->rhs->print(out);
    out << ")";
}

/*Depricated*/
//void Mult::pretty_print_at(std::ostream& out, print_mode_t mode, int num){
//    if(mode == print_group_add_or_mult){
//        out << "(";
//        this->lhs->pretty_print_at(out, print_group_none, 0);
//        out << " * ";
//        this->rhs->pretty_print_at(out, print_group_none, 0);
//        out << ")";
//    }
//    else {
//        this->lhs->pretty_print_at(out, print_group_add_or_mult, 0);
//        out << " * ";
//        this->rhs->pretty_print_at(out, print_group_add, 0);
//    }
//}

//Method Tests
TEST_CASE("Mult Test"){
    
    //Test Varialbels
        PTR(NumExpr) one = NEW(NumExpr)(1);
        PTR(NumExpr) two = NEW(NumExpr)(2);

        PTR(AddExpr) one_two = NEW(AddExpr)(one,two);
        PTR(AddExpr) two_one = NEW(AddExpr)(two,one);

        PTR(MultExpr) mult_one_two = NEW(MultExpr)(one,two);
        PTR(MultExpr) mult_two_one = NEW(MultExpr)(two,one);
        PTR(MultExpr) mult_two_mult_exper = NEW(MultExpr)(mult_one_two, mult_two_one);
        PTR(MultExpr) mult_two_add_exper = NEW(MultExpr)(one_two, two_one);
        PTR(MultExpr) vars = NEW(MultExpr)(NEW(VarExpr)("x"), NEW(NumExpr)(3));
        PTR(MultExpr) vars2 = NEW(MultExpr)(NEW(NumExpr)(8), NEW(VarExpr)("y"));
        PTR(MultExpr) vars3 = NEW(MultExpr)(one_two, NEW(MultExpr)(NEW(VarExpr)("z"), NEW(NumExpr)(7)));
        PTR(MultExpr) vars4 = NEW(MultExpr)(NEW(MultExpr)(NEW(VarExpr)("a"), NEW(NumExpr)(3)), two_one);
    
    
    //Checking Mult Equality
        CHECK(mult_one_two->equals(NEW(MultExpr)(NEW(NumExpr)(1), NEW(NumExpr)(2))));
    
    //Checking Mult Inequality
        CHECK(!(mult_one_two->equals(mult_two_one)));
    
    //Checking Class Inequality
        CHECK(!(mult_one_two->equals(one_two)));
        CHECK(!(mult_one_two->equals(one)));
    
    //Checking interp()
        CHECK(mult_one_two->interp()->equals(NEW(NumVal)(2)));
        CHECK(mult_two_add_exper->interp()->equals(NEW(NumVal)(9)));
        CHECK(mult_two_mult_exper->interp()->equals(NEW(NumVal)(4)));
        CHECK_THROWS_WITH(vars->interp(), "The variable has no value assigned to it yet.");
    
    //Checking has_varible()
        CHECK(vars->has_variable() == true);
        CHECK(vars2->has_variable() == true);
        CHECK(vars3->has_variable() == true);
        CHECK(vars4->has_variable() == true);
        CHECK(mult_two_mult_exper->has_variable() == false);
        CHECK(mult_two_add_exper->has_variable() == false);
    
    //Checking subst()
        //substitution with each avaiable Exper object.
            CHECK((NEW(MultExpr)(NEW(VarExpr)("x"), NEW(NumExpr)(7)))->subst("x", NEW(VarExpr)("y"))->equals(NEW(MultExpr)(NEW(VarExpr)("y"), NEW(NumExpr)(7))));
            CHECK((NEW(MultExpr)(NEW(VarExpr)("x"), NEW(NumExpr)(7)))->subst("x", NEW(NumExpr)(3))->equals(NEW(MultExpr)(NEW(NumExpr)(3), NEW(NumExpr)(7))));
            CHECK((NEW(MultExpr)(NEW(VarExpr)("x"), NEW(NumExpr)(7)))->subst("x", NEW(AddExpr)(NEW(VarExpr)("x"), NEW(NumExpr)(8)))->equals(NEW(MultExpr)(NEW(AddExpr)(NEW(VarExpr)("x"), NEW(NumExpr)(8)), NEW(NumExpr)(7))));
            CHECK((NEW(MultExpr)(NEW(VarExpr)("x"), NEW(NumExpr)(7)))->subst("x", NEW(MultExpr)(NEW(VarExpr)("x"), NEW(NumExpr)(8)))->equals(NEW(MultExpr)(NEW(MultExpr)(NEW(VarExpr)("x"), NEW(NumExpr)(8)), NEW(NumExpr)(7))));
        //nested substitution
            CHECK((NEW(MultExpr)(NEW(NumExpr)(3), NEW(MultExpr)(NEW(NumExpr)(6), NEW(VarExpr)("y"))))->subst("y", NEW(VarExpr)("x"))->equals(NEW(MultExpr)(NEW(NumExpr)(3), NEW(MultExpr)(NEW(NumExpr)(6), NEW(VarExpr)("x")))));
        //no Var object present
            CHECK((NEW(MultExpr)(NEW(NumExpr)(8), NEW(NumExpr)(7)))->subst("x", NEW(VarExpr)("y"))->equals(NEW(MultExpr)(NEW(NumExpr)(8), NEW(NumExpr)(7))));
        //no matching string
            CHECK((NEW(MultExpr)(NEW(VarExpr)("x"), NEW(NumExpr)(7)))->subst("mismatch", NEW(VarExpr)("y"))->equals(NEW(MultExpr)(NEW(VarExpr)("x"), NEW(NumExpr)(7))));
    
    //Checking print()
        //No nesting
            {
                std::stringstream rep_cout ("");
                (NEW(MultExpr)(NEW(NumExpr)(1), NEW(VarExpr)("x")))->print(rep_cout);
                CHECK(rep_cout.str() == "(1*x)");
            }
        //Nested right
            {
                std::stringstream rep_cout("");
                (NEW(MultExpr)(NEW(NumExpr)(3),NEW(MultExpr)(NEW(NumExpr)(1), NEW(VarExpr)("x"))))->print(rep_cout);
                CHECK(rep_cout.str() == "(3*(1*x))");
            }
        //Nested left
            {
                std::stringstream rep_cout("");
                (NEW(MultExpr)(NEW(MultExpr)(NEW(NumExpr)(1), NEW(VarExpr)("x")), NEW(NumExpr)(3)))->print(rep_cout);
                CHECK(rep_cout.str() == "((1*x)*3)");
            }
    
    /*Depricated*/
//    //Checking pretty_print()
//        //No nesting
//            {
//                std::stringstream rep_cout ("");
//                (new Mult(new Num(1), new Var("x")))->pretty_print(rep_cout);
//                CHECK(rep_cout.str() == "1 * x");
//            }
//        //Mult w/ Nested right Add
//            {
//                std::stringstream rep_cout("");
//                (new Mult(new Num(3),new Add(new Num(1), new Var("x"))))->pretty_print(rep_cout);
//                CHECK(rep_cout.str() == "3 * (1 + x)");
//            }
//        //Mult w/ Nested left Add
//            {
//                std::stringstream rep_cout("");
//                (new Mult(new Add(new Num(1), new Var("x")), new Num(3)))->pretty_print(rep_cout);
//                CHECK(rep_cout.str() == "(1 + x) * 3");
//            }
//        //Mult w/ Nested right Mult
//            {
//                std::stringstream rep_cout("");
//                (new Mult(new Num(3), new Mult(new Num(1), new Var("x"))))->pretty_print(rep_cout);
//                CHECK(rep_cout.str() == "3 * 1 * x");
//            }
//        //Mult w/ Nested left Mult
//            {
//                std::stringstream rep_cout("");
//                (new Mult(new Mult(new Num(1), new Var("x")), new Num(3)))->pretty_print(rep_cout);
//                CHECK(rep_cout.str() == "(1 * x) * 3");
//            }
//        //Mult w/ Nested left Mult
//            {
//                std::stringstream rep_cout("");
//                (new Mult(new Mult(new Num(1), new Var("x")), new Add(new Var("y"), new Num(5))))->pretty_print(rep_cout);
//                CHECK(rep_cout.str() == "(1 * x) * (y + 5)");
//            }
}






/*Var implementations*/
//Default Constructor
VarExpr::VarExpr(std::string val){
    this->var = val;
}

//Methods
bool VarExpr::equals(PTR(Expr)e){
    PTR(VarExpr)v = CAST(VarExpr)(e);
    if(v == NULL)
        return false;
    else
        return this->var == v->var;
}

PTR(Val) VarExpr::interp(){
    throw std::runtime_error("The variable has no value assigned to it yet.");
}

bool VarExpr::has_variable(){
    return true;
}

PTR(Expr) VarExpr::subst(std::string s, PTR(Expr)e){
    if(this->var == s)
        return e;
    else
        return THIS;
}

void VarExpr::print(std::ostream& out){
    out << this->var;
}


/*Depricated*/
//void Var::pretty_print_at(std::ostream& out, print_mode_t mode, int num){
//    print(out);
//}

//Method Tests
TEST_CASE("Var"){

    //Test Varialbels
        PTR(NumExpr) one = NEW(NumExpr)(1);
        PTR(NumExpr) two = NEW(NumExpr)(2);
        PTR(AddExpr) one_two = NEW(AddExpr)(one,two);
        PTR(MultExpr) mult_one_two = NEW(MultExpr)(one,two);
        PTR(VarExpr) first = NEW(VarExpr)("first");
        PTR(VarExpr) second = NEW(VarExpr)("second");

    //Checking Var Equality
        CHECK(first->equals(NEW(VarExpr)("first")));

    //Checking Var Inequality
        CHECK(!(first->equals(second)));
    
    //Checking Class Inequality
        CHECK(!(first->equals(one)));
        CHECK(!(first->equals(one_two)));
        CHECK(!(first->equals(mult_one_two)));
    
    //Checking interp()
        CHECK_THROWS_WITH(first->interp(), "The variable has no value assigned to it yet.");
        CHECK_THROWS_WITH(second->interp(), "The variable has no value assigned to it yet.");
    
    //Check has_variable()
        CHECK(first->has_variable() == true);
        CHECK(second->has_variable() == true);
    
    //Checking subst()
        CHECK(first->subst("first", NEW(AddExpr)(NEW(NumExpr)(3), NEW(VarExpr)("x")))->equals(NEW(AddExpr)(NEW(NumExpr)(3), NEW(VarExpr)("x"))));
        CHECK(first->subst("second", NEW(VarExpr)("second"))->equals(NEW(VarExpr)("first")));
    
    //Checking print()
        {
            std::stringstream rep_cout ("");
            (NEW(VarExpr)("x"))->print(rep_cout);
            CHECK(rep_cout.str() == "x");
        }
    
    /*Depricated*/
    //Checking pretty_print()
//        {
//            std::stringstream rep_cout ("");
//            (new Var("x"))->pretty_print(rep_cout);
//            CHECK(rep_cout.str() == "x");
//        }
}






/*LetExpr implementations*/
//Default Constructor
LetExpr::LetExpr(std::string lhs_name, PTR(Expr)rhs,PTR(Expr)body){
    this->lhs_name = lhs_name;
    this->rhs = rhs;
    this->body = body;
}

//Methods
bool LetExpr::equals(PTR(Expr)e){
    PTR(LetExpr)l = CAST(LetExpr)(e);
    if(l == NULL)
        return false;
    else
        return this->lhs_name == l->lhs_name && this->rhs->equals(l->rhs) && this->body->equals(l->body);
}

PTR(Val) LetExpr::interp(){
    return this->body->subst(this->lhs_name, rhs->interp()->to_expr())->interp();
}

bool LetExpr::has_variable(){
    return this->rhs->has_variable() || this->body->has_variable();
}

PTR(Expr) LetExpr::subst(std::string s, PTR(Expr)e){
    if(this->lhs_name != s){
        return NEW(LetExpr)(this->lhs_name, this->rhs->subst(s, e), this->body->subst(s, e));
    }
    return NEW(LetExpr)(this->lhs_name, this->rhs->subst(s, e), this->body);
}

void LetExpr::print(std::ostream& out){
    out << "(_let " << this->lhs_name << "=";
    this->rhs->print(out);
    out << " _in ";
    this->body->print(out);
    out << ")";
}

/*Depricated*/
//void _let::pretty_print_at(std::ostream& out, print_mode_t mode, int num){
//    out << "_let " << this->lhs_name << " = ";
//    this->rhs->pretty_print_at(out, print_group_none, 0);
//    out << "\n" << "_in ";
//    this->body->pretty_print_at(out, print_group_add, 0);
//}

//Method Tests
TEST_CASE("_let"){
    PTR(LetExpr) firstExpression = NEW(LetExpr)("x", NEW(NumExpr)(7), NEW(MultExpr)(NEW(VarExpr)("x"), NEW(NumExpr)(7)));
    PTR(LetExpr) secondExpression = NEW(LetExpr)("x", NEW(NumExpr)(7), NEW(LetExpr)("x", NEW(NumExpr)(8), NEW(AddExpr)(NEW(VarExpr)("x"), NEW(NumExpr)(7))));
    PTR(LetExpr) thridExpression = NEW(LetExpr)("x", NEW(NumExpr)(7), NEW(LetExpr)("x", NEW(AddExpr)(NEW(VarExpr)("x"), NEW(NumExpr)(8)), NEW(AddExpr)(NEW(VarExpr)("x"), NEW(NumExpr)(7))));
    PTR(LetExpr) fourthExpression = NEW(LetExpr)("x", NEW(NumExpr)(7), NEW(LetExpr)("y", NEW(NumExpr)(8), NEW(AddExpr)(NEW(VarExpr)("x"), NEW(NumExpr)(7))));
    PTR(LetExpr) invalidExpression_body_free_var = NEW(LetExpr)("x", NEW(NumExpr)(7), NEW(LetExpr)("x", NEW(AddExpr)(NEW(VarExpr)("x"), NEW(NumExpr)(8)), NEW(AddExpr)(NEW(VarExpr)("y"), NEW(NumExpr)(7))));
    PTR(LetExpr) invalidExpression_rhs_free_var = NEW(LetExpr)("y", NEW(VarExpr)("y"), NEW(AddExpr)(NEW(NumExpr)(3), NEW(VarExpr)("y")));
    PTR(LetExpr) noVariablePresent =NEW(LetExpr)("x", NEW(NumExpr)(7), NEW(MultExpr)(NEW(NumExpr)(5), NEW(NumExpr)(7)));
    
    //print variables
    PTR(LetExpr) print_test = NEW(LetExpr)("x", NEW(NumExpr)(5), NEW(AddExpr)(NEW(LetExpr)("y", NEW(NumExpr)(3), NEW(AddExpr)(NEW(VarExpr)("y"), NEW(NumExpr)(2))), NEW(VarExpr)("x")));
    
    //Checking _let Equality
    CHECK(firstExpression->equals(NEW(LetExpr)("x", NEW(NumExpr)(7), NEW(MultExpr)(NEW(VarExpr)("x"), NEW(NumExpr)(7)))));

    //Checking _let Inequality
    CHECK(!(firstExpression->equals(NEW(LetExpr)("y", NEW(NumExpr)(7), NEW(MultExpr)(NEW(VarExpr)("x"), NEW(NumExpr)(7))))));
    CHECK(!(firstExpression->equals(NEW(LetExpr)("x", NEW(NumExpr)(8), NEW(MultExpr)(NEW(VarExpr)("x"), NEW(NumExpr)(7))))));
    CHECK(!(firstExpression->equals(NEW(LetExpr)("x", NEW(NumExpr)(7), NEW(MultExpr)(NEW(VarExpr)("y"), NEW(NumExpr)(7))))));
    CHECK(!(firstExpression->equals(NEW(LetExpr)("x", NEW(NumExpr)(7), NEW(MultExpr)(NEW(VarExpr)("x"), NEW(NumExpr)(8))))));
    CHECK(!(firstExpression->equals(NEW(LetExpr)("x", NEW(NumExpr)(7), NEW(MultExpr)(NEW(NumExpr)(7), NEW(VarExpr)("x"))))));
    
    //Checking Class Inequality
    CHECK(!firstExpression->equals(NEW(VarExpr)("x")));
    CHECK(!firstExpression->equals(NEW(NumExpr)(7)));
    CHECK(!firstExpression->equals(NEW(AddExpr)(NEW(NumExpr)(0), NEW(NumExpr)(0))));
    CHECK(!firstExpression->equals(NEW(MultExpr)(NEW(NumExpr)(0), NEW(NumExpr)(0))));
    
    //Checking subst()
    // _let x = 1
    // _in  x + 2  -> subst("x", y+3)
    // ==
    // _let x = 1
    // _in  x + 2
    PTR(Expr) noSubstitutionNeeded = NEW(LetExpr)("x", NEW(NumExpr)(1),NEW(AddExpr)(NEW(VarExpr)("x"), NEW(NumExpr)(2)));
    CHECK(noSubstitutionNeeded->subst("x", NEW(AddExpr)(NEW(VarExpr)("y"), NEW(NumExpr)(3)))->equals(NEW(LetExpr)("x", NEW(NumExpr)(1),NEW(AddExpr)(NEW(VarExpr)("x"), NEW(NumExpr)(2)))));
    
    // _let z = x
    // _in  z + 2 -> subst("z", 0)
    // =
    // _let z = x
    // _in  z + 2
    PTR(Expr) noSubstitutionNeeded_DifferentVariable = NEW(LetExpr)("z", NEW(VarExpr)("x"), NEW(AddExpr)(NEW(VarExpr)("z"), NEW(NumExpr)(2)));
    CHECK(noSubstitutionNeeded_DifferentVariable->subst("z", NEW(NumExpr)(0))->equals(NEW(LetExpr)("z", NEW(VarExpr)("x"), NEW(AddExpr)(NEW(VarExpr)("z"), NEW(NumExpr)(2)))));
    
    // _let x = x
    // _in  x + 2  -> subst("x", y+3)
    // ==
    // _let x = y+3
    // _in  x + 2
    PTR(Expr) subOnlyTheRHS_SameVariable = NEW(LetExpr)("x", NEW(VarExpr)("x"), NEW(AddExpr)(NEW(VarExpr)("x"), NEW(NumExpr)(2)));
    CHECK(subOnlyTheRHS_SameVariable->subst("x", NEW(AddExpr)(NEW(VarExpr)("y"), NEW(NumExpr)(3)))->equals(NEW(LetExpr)("x", NEW(AddExpr)(NEW(VarExpr)("y"), NEW(NumExpr)(3)), NEW(AddExpr)(NEW(VarExpr)("x"), NEW(NumExpr)(2)))));
    
    // _let x = y
    // _in  x + 2 -> subst("y", 8)
    // =
    // _let x = 8
    // _in  x + 2
    PTR(Expr) subOnlyTheRHS_DifferentVariable = NEW(LetExpr)("x", NEW(VarExpr)("y"), NEW(AddExpr)(NEW(VarExpr)("x"), NEW(NumExpr)(2)));
    CHECK(subOnlyTheRHS_DifferentVariable->subst("y", NEW(NumExpr)(8))->equals(NEW(LetExpr)("x", NEW(NumExpr)(8), NEW(AddExpr)(NEW(VarExpr)("x"), NEW(NumExpr)(2)))));
    
    // _let z = z + 2
    // _in  z + 2 -> subst("z", 0)
    // =
    // _let z = 0 + 2
    // _in  z + 2
    PTR(Expr) subOnlyRHS_Addition = NEW(LetExpr)("z", NEW(AddExpr)(NEW(VarExpr)("z"), NEW(NumExpr)(2)), NEW(AddExpr)(NEW(VarExpr)("z"), NEW(NumExpr)(2)));
    CHECK(subOnlyRHS_Addition->subst("z", NEW(NumExpr)(0))->equals(NEW(LetExpr)("z", NEW(AddExpr)(NEW(NumExpr)(0), NEW(NumExpr)(2)), NEW(AddExpr)(NEW(VarExpr)("z"), NEW(NumExpr)(2)))));
    
    // _let x = y
    // _in  x + y -> subst("y", 8)
    // ==
    // _let x = 8
    // _in  x + 8
    PTR(Expr) subTheRHSAndBody = NEW(LetExpr)("x", NEW(VarExpr)("y"), NEW(AddExpr)(NEW(VarExpr)("x"), NEW(VarExpr)("y")));
    CHECK(subTheRHSAndBody->subst("y", NEW(NumExpr)(8))->equals(NEW(LetExpr)("x", NEW(NumExpr)(8), NEW(AddExpr)(NEW(VarExpr)("x"), NEW(NumExpr)(8)))));
    
    // _let x = 8
    // _in  x + 2 + y -> subst("y", 9)
    // =
    // _let x = 8
    // _in  x + 2 + 9
    PTR(Expr) subWithinBodyOnly = NEW(LetExpr)("x", NEW(NumExpr)(8), NEW(AddExpr)(NEW(VarExpr)("x"), NEW(AddExpr)(NEW(NumExpr)(2), NEW(VarExpr)("y"))));
    CHECK(subWithinBodyOnly->subst("y", NEW(NumExpr)(9))->equals(NEW(LetExpr)("x", NEW(NumExpr)(8), NEW(AddExpr)(NEW(VarExpr)("x"), NEW(AddExpr)(NEW(NumExpr)(2), NEW(NumExpr)(9))))));
    
    
    //Checking interp()
    CHECK(firstExpression->interp()->equals(NEW(NumVal)(49)));
    CHECK(secondExpression->interp()->equals(NEW(NumVal)(15)));
    CHECK(thridExpression->interp()->equals(NEW(NumVal)(22)));
    CHECK(fourthExpression->interp()->equals(NEW(NumVal)(14)));
    CHECK(noVariablePresent->interp()->equals(NEW(NumVal)(35)));
    CHECK_THROWS_WITH(invalidExpression_rhs_free_var->interp(), "The variable has no value assigned to it yet.");
    CHECK_THROWS_WITH(invalidExpression_body_free_var->interp(), "The variable has no value assigned to it yet.");
    
    //Check has_variable()
    CHECK(firstExpression->has_variable());
    CHECK(secondExpression->has_variable());
    CHECK(thridExpression->has_variable());
    CHECK(invalidExpression_rhs_free_var->has_variable());
    CHECK(invalidExpression_body_free_var->has_variable());
    CHECK(!noVariablePresent->has_variable());
        
    
    //Checking print()
    CHECK(firstExpression->to_string() == "(_let x=7 _in (x*7))");
    CHECK(print_test->to_string() == "(_let x=5 _in ((_let y=3 _in (y+2))+x))");
    
    /*Depricated*/
//    //Checking pretty_print()
//    Add* exampleTests = new Add(new Mult(new Num(5), new _let("x", new Num(5), new Var("x"))), new Num(1));
//    {
//        std::stringstream rep_cout ("");
//        print_test->pretty_print(rep_cout);
//        CHECK(rep_cout.str() == "_let x = 5\n_in  (_let y = 3\n      _in  y + 2) + x");
//    }
}





/* BoolExpr Implementation */
//Default Constructor
BoolExpr::BoolExpr(bool rep){
    this->rep = rep;
}

//Methods
bool BoolExpr::equals(PTR(Expr)e){
    PTR(BoolExpr)be = CAST(BoolExpr)(e);
    if(be == NULL)
        return false;
    else
        return this->rep == be->rep;
}

PTR(Val) BoolExpr::interp(){
    return NEW(BoolVal)(this->rep);
}

bool BoolExpr::has_variable(){
    return false;
}

PTR(Expr) BoolExpr::subst(std::string s, PTR(Expr)e){
    return THIS;
}

void BoolExpr::print(std::ostream& out){
    if (this->rep)
        out << "_true";
    else
        out << "_false";
}


TEST_CASE("BoolExpr Tests"){
    /* equals() */
    CHECK((NEW(BoolExpr)(false))->equals(NEW(BoolExpr)(false)));
    CHECK(!((NEW(BoolExpr)(false))->equals(NEW(BoolExpr)(true))));
    CHECK(!((NEW(BoolExpr)(false))->equals(NEW(NumExpr)(4))));
    
    /* interp() */
    CHECK((NEW(BoolExpr)(true))->interp()->equals(NEW(BoolVal)(true)));
    
    /* has_variable() */
    CHECK(!((NEW(BoolExpr)(true))->has_variable()));
    
    /* subst() */
    CHECK((NEW(BoolExpr)(true))->subst("s", NEW(NumExpr)(3))->equals(NEW(BoolExpr)(true)));
    
    /* print */
    {
        std::stringstream rep_cout ("");
        (NEW(BoolExpr)(true))->print(rep_cout);
        CHECK(rep_cout.str() == "_true");
    }
    {
        std::stringstream rep_cout ("");
        (NEW(BoolExpr)(false))->print(rep_cout);
        CHECK(rep_cout.str() == "_false");
    }
}





/* EqExpr Implementation */
//Default Constructor
EqExpr::EqExpr(PTR(Expr) lhs, PTR(Expr) rhs){
    this->lhs = lhs;
    this->rhs = rhs;
}

//Methods
bool EqExpr::equals(PTR(Expr)e){
    PTR(EqExpr) ee = CAST(EqExpr)(e);
    if (ee == NULL)
        return false;
    else
        return (this->lhs->equals(ee->lhs) && this->rhs->equals(ee->rhs));
}

PTR(Val) EqExpr::interp(){
    if (this->lhs->interp()->equals(this->rhs->interp()))
        return NEW(BoolVal)(true);
    else
        return NEW(BoolVal)(false);
}

bool EqExpr::has_variable(){
    return this->lhs->has_variable() || this->rhs->has_variable();
}

PTR(Expr) EqExpr::subst(std::string s, PTR(Expr) e){
    return NEW(EqExpr)(this->lhs->subst(s, e), this->rhs->subst(s, e));
}

void EqExpr::print(std::ostream& out){
    out << "(";
    this->lhs->print(out);
    out << "==";
    this->rhs->print(out);
    out << ")";
}

TEST_CASE("EqExpr Tests"){
    /* equals() */
    CHECK((NEW(EqExpr)(NEW(NumExpr)(0), NEW(NumExpr)(1)))->equals(NEW(EqExpr)(NEW(NumExpr)(0), NEW(NumExpr)(1))));
    CHECK(!((NEW(EqExpr)(NEW(NumExpr)(0), NEW(NumExpr)(0)))->equals(NEW(EqExpr)(NEW(NumExpr)(0), NEW(NumExpr)(1)))));
    CHECK(!((NEW(EqExpr)(NEW(NumExpr)(1), NEW(NumExpr)(1)))->equals(NEW(EqExpr)(NEW(NumExpr)(0), NEW(NumExpr)(1)))));
    CHECK(!((NEW(EqExpr)(NEW(NumExpr)(1), NEW(NumExpr)(1)))->equals(NEW(NumExpr)(0))));
    
    /* interp() */
    CHECK((NEW(EqExpr)(NEW(BoolExpr)(false), NEW(BoolExpr)(false)))->interp()->equals(NEW(BoolVal)(true)));
    CHECK((NEW(EqExpr)(NEW(BoolExpr)(false), NEW(BoolExpr)(true)))->interp()->equals(NEW(BoolVal)(false)));
    CHECK((NEW(EqExpr)(NEW(NumExpr)(1), NEW(BoolExpr)(true)))->interp()->equals(NEW(BoolVal)(false)));
    CHECK((NEW(EqExpr)(NEW(BoolExpr)(false), NEW(NumExpr)(0)))->interp()->equals(NEW(BoolVal)(false)));
    
    
    /* has_variable() */
    CHECK((NEW(EqExpr)(NEW(VarExpr)("x"), NEW(NumExpr)(1)))->has_variable());
    CHECK((NEW(EqExpr)(NEW(NumExpr)(1), NEW(VarExpr)("x")))->has_variable());
    CHECK(!((NEW(EqExpr)(NEW(NumExpr)(1), NEW(BoolExpr)(true)))->has_variable()));
    
    /* subst() */
    CHECK((NEW(EqExpr)(NEW(VarExpr)("x"), NEW(NumExpr)(1)))->subst("x", NEW(NumExpr)(5))->equals(NEW(EqExpr)(NEW(NumExpr)(5), NEW(NumExpr)(1))));
    CHECK((NEW(EqExpr)(NEW(NumExpr)(1), NEW(VarExpr)("x")))->subst("x", NEW(NumExpr)(5))->equals(NEW(EqExpr)(NEW(NumExpr)(1), NEW(NumExpr)(5))));
    CHECK((NEW(EqExpr)(NEW(NumExpr)(1), NEW(NumExpr)(6)))->subst("x", NEW(NumExpr)(5))->equals(NEW(EqExpr)(NEW(NumExpr)(1), NEW(NumExpr)(6))));
    CHECK((NEW(EqExpr)(NEW(NumExpr)(1), NEW(VarExpr)("y")))->subst("x", NEW(NumExpr)(5))->equals(NEW(EqExpr)(NEW(NumExpr)(1), NEW(VarExpr)("y"))));
    CHECK((NEW(EqExpr)(NEW(VarExpr)("y"), NEW(NumExpr)(1)))->subst("x", NEW(NumExpr)(5))->equals(NEW(EqExpr)(NEW(VarExpr)("y"), NEW(NumExpr)(1))));
    
    /* print() */
    {
        std::stringstream rep_cout ("");
        (NEW(EqExpr)(NEW(VarExpr)("x"), NEW(NumExpr)(1)))->print(rep_cout);
        CHECK(rep_cout.str() == "(x==1)");
    }
    {
        std::stringstream rep_cout ("");
        (NEW(EqExpr)(NEW(VarExpr)("7"), NEW(BoolExpr)(false)))->print(rep_cout);
        CHECK(rep_cout.str() == "(7==_false)");
    }
}





/* IfExpr Implementation */
//Default Constructor
IfExpr::IfExpr(PTR(Expr) comparison, PTR(Expr) if_true, PTR(Expr) if_false){
    this->comparison = comparison;
    this->if_true = if_true;
    this->if_false = if_false;
}

//Methods
bool IfExpr::equals(PTR(Expr)e){
    PTR(IfExpr) ife = CAST(IfExpr)(e);
    if (ife == NULL)
        return false;
    else
        return this->comparison->equals(ife->comparison) && this->if_true->equals(ife->if_true) && this->if_false->equals(ife->if_false);
}

PTR(Val) IfExpr::interp(){
    if(this->comparison->interp()->is_true())
        return this->if_true->interp();
    else
        return this->if_false->interp();
    
}

bool IfExpr::has_variable(){
    return this->comparison->has_variable() || this->if_true->has_variable() || this->if_false->has_variable();
}
PTR(Expr) IfExpr::subst(std::string s, PTR(Expr)e){
    return NEW(IfExpr)(comparison->subst(s, e), if_true->subst(s, e), if_false->subst(s, e));
}

void IfExpr::print(std::ostream& out){
    out << "(_if ";
    this->comparison->print(out);
    out << "_then ";
    this->if_true->print(out);
    out << "_else ";
    this->if_false->print(out);
    out << ")";
    
}


TEST_CASE("IfExpr Tests"){
    /* equals() */
    CHECK((NEW(IfExpr)(NEW(NumExpr)(0), NEW(NumExpr)(0) , NEW(NumExpr)(0)))->equals(NEW(IfExpr)(NEW(NumExpr)(0), NEW(NumExpr)(0) , NEW(NumExpr)(0))));
    CHECK(!((NEW(IfExpr)(NEW(NumExpr)(0), NEW(NumExpr)(0) , NEW(NumExpr)(0)))->equals(NEW(NumExpr)(0))));
    CHECK(!((NEW(IfExpr)(NEW(NumExpr)(1), NEW(NumExpr)(0) , NEW(NumExpr)(0)))->equals(NEW(IfExpr)(NEW(NumExpr)(0), NEW(NumExpr)(0) , NEW(NumExpr)(0)))));
    CHECK(!((NEW(IfExpr)(NEW(NumExpr)(0), NEW(NumExpr)(1) , NEW(NumExpr)(0)))->equals(NEW(IfExpr)(NEW(NumExpr)(0), NEW(NumExpr)(0) , NEW(NumExpr)(0)))));
    CHECK(!((NEW(IfExpr)(NEW(NumExpr)(0), NEW(NumExpr)(0) , NEW(NumExpr)(1)))->equals(NEW(IfExpr)(NEW(NumExpr)(0), NEW(NumExpr)(0) , NEW(NumExpr)(0)))));
    CHECK(!((NEW(IfExpr)(NEW(VarExpr)("x"), NEW(NumExpr)(0) , NEW(NumExpr)(0)))->equals(NEW(IfExpr)(NEW(NumExpr)(0), NEW(NumExpr)(0) , NEW(NumExpr)(0)))));
    CHECK(!((NEW(IfExpr)(NEW(NumExpr)(0), NEW(VarExpr)("x") , NEW(NumExpr)(0)))->equals(NEW(IfExpr)(NEW(NumExpr)(0), NEW(NumExpr)(0) , NEW(NumExpr)(0)))));
    CHECK(!((NEW(IfExpr)(NEW(NumExpr)(0), NEW(NumExpr)(0) , NEW(VarExpr)("x")))->equals(NEW(IfExpr)(NEW(NumExpr)(0), NEW(NumExpr)(0) , NEW(NumExpr)(0)))));
    
    /* interp() */
    CHECK((NEW(IfExpr)(NEW(BoolExpr)(true), NEW(NumExpr)(1), NEW(NumExpr)(0)))->interp()->equals(NEW(NumVal)(1)));
    CHECK((NEW(IfExpr)(NEW(BoolExpr)(false), NEW(NumExpr)(1), NEW(NumExpr)(0)))->interp()->equals(NEW(NumVal)(0)));
    
    /* has_variable() */
    CHECK(!((NEW(IfExpr)(NEW(NumExpr)(0), NEW(NumExpr)(0) , NEW(NumExpr)(0)))->has_variable()));
    CHECK((NEW(IfExpr)(NEW(VarExpr)("x"), NEW(NumExpr)(0) , NEW(NumExpr)(0)))->has_variable());
    CHECK((NEW(IfExpr)(NEW(NumExpr)(0), NEW(VarExpr)("x") , NEW(NumExpr)(0)))->has_variable());
    CHECK((NEW(IfExpr)(NEW(NumExpr)(0), NEW(NumExpr)(0) , NEW(VarExpr)("x")))->has_variable());
    
    /* subst() */
    CHECK((NEW(IfExpr)(NEW(NumExpr)(0), NEW(NumExpr)(0) , NEW(NumExpr)(0)))->subst("x", NEW(NumExpr)(5))->equals(NEW(IfExpr)(NEW(NumExpr)(0), NEW(NumExpr)(0) , NEW(NumExpr)(0))));
    CHECK((NEW(IfExpr)(NEW(VarExpr)("x"), NEW(NumExpr)(0) , NEW(NumExpr)(0)))->subst("x", NEW(NumExpr)(5))->equals(NEW(IfExpr)(NEW(NumExpr)(5), NEW(NumExpr)(0) , NEW(NumExpr)(0))));
    CHECK((NEW(IfExpr)(NEW(NumExpr)(0), NEW(VarExpr)("x") , NEW(NumExpr)(0)))->subst("x", NEW(NumExpr)(5))->equals(NEW(IfExpr)(NEW(NumExpr)(0), NEW(NumExpr)(5) , NEW(NumExpr)(0))));
    CHECK((NEW(IfExpr)(NEW(NumExpr)(0), NEW(NumExpr)(0) , NEW(VarExpr)("x")))->subst("x", NEW(NumExpr)(5))->equals(NEW(IfExpr)(NEW(NumExpr)(0), NEW(NumExpr)(0) , NEW(NumExpr)(5))));
    
    /* print() */
    {
        std::stringstream rep_cout ("");
        (NEW(IfExpr)(NEW(NumExpr)(2), NEW(NumExpr)(0) , NEW(VarExpr)("x")))->print(rep_cout);
        CHECK(rep_cout.str() == "(_if 2_then 0_else x)");
    }
    {
        std::stringstream rep_cout ("");
        (NEW(IfExpr)(NEW(NumExpr)(5), NEW(NumExpr)(6) , NEW(NumExpr)(4)))->print(rep_cout);
        CHECK(rep_cout.str() == "(_if 5_then 6_else 4)");
    }
}

/* FunExpr Implementations */
//Default Constructor
FunExpr::FunExpr(std::string formal_arg, PTR(Expr) body){
    this->formal_arg = formal_arg;
    this->body = body;
}

//Methods
bool FunExpr::equals(PTR(Expr) e){
    PTR(FunExpr) fe = CAST(FunExpr)(e);
    if (fe == NULL)
        return false;
    else
        return this->formal_arg == fe->formal_arg && this->body->equals(fe->body);
}

PTR(Val) FunExpr::interp(){
    return NEW(FunVal)(this->formal_arg, this->body);
}

bool FunExpr::has_variable(){
    return false;
}

PTR(Expr) FunExpr::subst(std::string s, PTR(Expr) e){
    if(this->formal_arg != s){
        return NEW(FunExpr)(this->formal_arg, this->body->subst(s, e));
    }
    return NEW(FunExpr)(this->formal_arg, this->body);
}

void FunExpr::print(std::ostream& out){
    out << "(_fun (" + this->formal_arg + ") ";
    this->body->print(out);
    out << ")";
}

TEST_CASE("FunExpr Tests"){
    /* equals() */
    CHECK((NEW(FunExpr)("x", NEW(AddExpr)(NEW(VarExpr)("x"), NEW(NumExpr)(4))))->equals(NEW(FunExpr)("x", NEW(AddExpr)(NEW(VarExpr)("x"), NEW(NumExpr)(4)))));
    CHECK(!((NEW(FunExpr)("x", NEW(AddExpr)(NEW(VarExpr)("x"), NEW(NumExpr)(4))))->equals(NEW(FunExpr)("y", NEW(AddExpr)(NEW(VarExpr)("x"), NEW(NumExpr)(4))))));
    CHECK(!((NEW(FunExpr)("x", NEW(AddExpr)(NEW(VarExpr)("x"), NEW(NumExpr)(4))))->equals(NEW(FunExpr)("x", NEW(AddExpr)(NEW(VarExpr)("z"), NEW(NumExpr)(4))))));
    CHECK(!((NEW(FunExpr)("x", NEW(AddExpr)(NEW(VarExpr)("x"), NEW(NumExpr)(4))))->equals(NEW(BoolExpr)(true))));
    
    /* interp() */
    CHECK((NEW(FunExpr)("x", NEW(AddExpr)(NEW(VarExpr)("x"), NEW(NumExpr)(4))))->interp()->equals(NEW(FunVal)("x", NEW(AddExpr)(NEW(VarExpr)("x"), NEW(NumExpr)(4)))));
    
    /* has_variable() */
    CHECK(!((NEW(FunExpr)("x", NEW(AddExpr)(NEW(VarExpr)("x"), NEW(NumExpr)(4))))->has_variable()));
    
    /* subst() */
    //_fun(x) x + y->subst(x, 4) == _fun(x) x + y
    CHECK((NEW(FunExpr)("x", NEW(AddExpr)(NEW(VarExpr)("x"), NEW(VarExpr)("y"))))->subst("x", NEW(NumExpr)(4))->equals(NEW(FunExpr)("x", NEW(AddExpr)(NEW(VarExpr)("x"), NEW(VarExpr)("y")))));
    //_fun(x) x + y->subst(y, 4) == _fun(x) x + 4
    CHECK((NEW(FunExpr)("x", NEW(AddExpr)(NEW(VarExpr)("x"), NEW(VarExpr)("y"))))->subst("y", NEW(NumExpr)(4))->equals(NEW(FunExpr)("x", NEW(AddExpr)(NEW(VarExpr)("x"), NEW(NumExpr)(4)))));
    
//        _let factrl = _fun (factrl)
//                        _fun (x)
//                          _if x == 1
//                          _then 1
//                          _else x * factrl(factrl)(x + -1)
//        _in  factrl(factrl)(10)

    /* print() */
    {
        std::stringstream rep_cout ("");
        (NEW(FunExpr)("x", NEW(AddExpr)(NEW(VarExpr)("x"), NEW(NumExpr)(4))))->print(rep_cout);
        CHECK(rep_cout.str() == "(_fun (x) (x+4))");
    }
}

/* CallExpr Implementations */
//Default Constructor
CallExpr::CallExpr(PTR(Expr) to_be_called, PTR(Expr) actual_arg){
    this->to_be_called = to_be_called;
    this->actual_arg = actual_arg;
}

//Methods
bool CallExpr::equals(PTR(Expr)e){
    PTR(CallExpr) ce = CAST(CallExpr)(e);
    if (ce == NULL)
        return false;
    else
        return this->to_be_called->equals(ce->to_be_called) && this->actual_arg->equals(ce->actual_arg);
}

PTR(Val) CallExpr::interp(){
    return to_be_called->interp()->call(actual_arg->interp());
}

bool CallExpr::has_variable(){
    return false;
}

PTR(Expr) CallExpr::subst(std::string s, PTR(Expr)e){
    //interp before subst
    return NEW(CallExpr)(this->to_be_called->subst(s, e), this->actual_arg->subst(s, e));
}

void CallExpr::print(std::ostream& out){
    this->to_be_called->print(out);
    out << "(";
    this->actual_arg->print(out);
    out << ")";
}

TEST_CASE("CallExpr Tests"){
    /* equals() */
    CHECK((NEW(CallExpr)(NEW(VarExpr)("x"), NEW(MultExpr)(NEW(VarExpr)("x"), NEW(NumExpr)(4))))->equals(NEW(CallExpr)(NEW(VarExpr)("x"), NEW(MultExpr)(NEW(VarExpr)("x"), NEW(NumExpr)(4)))));
    CHECK(!((NEW(CallExpr)(NEW(VarExpr)("x"), NEW(MultExpr)(NEW(VarExpr)("x"), NEW(NumExpr)(4))))->equals(NEW(CallExpr)(NEW(VarExpr)("y"), NEW(MultExpr)(NEW(VarExpr)("x"), NEW(NumExpr)(4))))));
    CHECK(!((NEW(CallExpr)(NEW(VarExpr)("x"), NEW(MultExpr)(NEW(VarExpr)("x"), NEW(NumExpr)(4))))->equals(NEW(CallExpr)(NEW(VarExpr)("x"), NEW(MultExpr)(NEW(VarExpr)("y"), NEW(NumExpr)(4))))));
    CHECK(!((NEW(CallExpr)(NEW(VarExpr)("x"), NEW(MultExpr)(NEW(VarExpr)("x"), NEW(NumExpr)(4))))->equals(NEW(CallExpr)(NEW(BoolExpr)(true), NEW(MultExpr)(NEW(VarExpr)("x"), NEW(NumExpr)(4))))));
    CHECK(!((NEW(CallExpr)(NEW(VarExpr)("x"), NEW(MultExpr)(NEW(VarExpr)("x"), NEW(NumExpr)(4))))->equals(NEW(BoolExpr)(true))));
    
    /* interp() */
    CHECK((NEW(CallExpr)(NEW(FunExpr)("x", NEW(MultExpr)(NEW(VarExpr)("x"), NEW(NumExpr)(4))), NEW(NumExpr)(4)))->interp()->equals(NEW(NumVal)(16)));
    
    // (_fun(x) (_fun(x) x + 1)(2) + x)(5)
    CHECK((NEW(CallExpr)(NEW(FunExpr)("x", NEW(AddExpr)(NEW(CallExpr)(NEW(FunExpr)("x", NEW(AddExpr)(NEW(VarExpr)("x"),NEW(NumExpr)(1))),NEW(NumExpr)(2)), NEW(VarExpr)("x"))), NEW(NumExpr)(5)))->interp()->equals(NEW(NumVal)(8)));
    
    /* has_variable() */
    CHECK(!((NEW(CallExpr)(NEW(FunExpr)("x", NEW(MultExpr)(NEW(VarExpr)("x"), NEW(NumExpr)(4))), NEW(NumExpr)(4)))->has_variable()));
    
    /* subst() */
    CHECK((NEW(CallExpr)(NEW(VarExpr)("f"), NEW(NumExpr)(5)))->subst("f", NEW(FunExpr)("x", NEW(AddExpr)(NEW(VarExpr)("x"), NEW(NumExpr)(1))))->equals(NEW(CallExpr)(NEW(FunExpr)("x", NEW(AddExpr)(NEW(VarExpr)("x"), NEW(NumExpr)(1))), NEW(NumExpr)(5))));
    CHECK((NEW(CallExpr)(NEW(VarExpr)("x"), NEW(NumExpr)(5)))->subst("f", NEW(FunExpr)("x", NEW(AddExpr)(NEW(VarExpr)("x"), NEW(NumExpr)(1))))->equals(NEW(CallExpr)(NEW(VarExpr)("x"), NEW(NumExpr)(5))));
    CHECK((NEW(CallExpr)(NEW(VarExpr)("x"), NEW(AddExpr)(NEW(NumExpr)(4), NEW(VarExpr)("y"))))->subst("y", NEW(NumExpr)(4))->equals(NEW(CallExpr)(NEW(VarExpr)("x"), NEW(AddExpr)(NEW(NumExpr)(4),NEW(NumExpr)(4)))));
    PTR(FunExpr) functionReturnfunction = NEW(FunExpr)("x", NEW(FunExpr)("y", NEW(AddExpr)(NEW(VarExpr)("x"), NEW(VarExpr)("y"))));
    CHECK((NEW(CallExpr)(NEW(CallExpr)(NEW(VarExpr)("f"), NEW(NumExpr)(5)), NEW(NumExpr)(1)))->subst("f", functionReturnfunction)->equals(NEW(CallExpr)(NEW(CallExpr)(NEW(FunExpr)("x", NEW(FunExpr)("y", NEW(AddExpr)(NEW(VarExpr)("x"), NEW(VarExpr)("y")))), NEW(NumExpr)(5)), NEW(NumExpr)(1))));
    
    /* print() */
    {
        std::stringstream rep_cout ("");
        (NEW(CallExpr)(NEW(VarExpr)("f"), NEW(NumExpr)(4)))->print(rep_cout);
        CHECK(rep_cout.str() == "f(4)");
    }
    
}
