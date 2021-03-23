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
    CHECK((new MultExpr(new AddExpr(new NumExpr(0), new VarExpr("y")), new NumExpr(1)))->to_string() == "((0+y)*1)");
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

Expr* parse_num(std::istream &to_Parse){
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
    
    return new NumExpr(to_Return);
}

TEST_CASE("parse_num"){
    {
        std::stringstream testing ("33");
        CHECK(parse_num(testing)->equals(new NumExpr(33)));
    }
    
    {
        std::stringstream testing ("-33");
        CHECK(parse_num(testing)->equals(new NumExpr(-33)));
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

Expr* parse_var(std::istream &to_Parse){
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
    return new VarExpr(to_Return);
}

TEST_CASE("parse_var"){
    {
        std::stringstream testing ("g");
        CHECK(parse_var(testing)->equals(new VarExpr("g")));
    }
    
    {
        std::stringstream testing ("guess0");
        CHECK(parse_var(testing)->equals(new VarExpr("guess")));
    }
    
    {
        std::stringstream testing ("gu0ess");
        CHECK(parse_var(testing)->equals(new VarExpr("gu")));
    }
    
    {
        std::stringstream testing ("0guess");
        CHECK(parse_var(testing)->equals(new VarExpr("")));
    }
}

Expr* parse_let(std::istream &to_Parse){
    parse_keyword(to_Parse, "let");
    skip_whitespace(to_Parse);
    std::string lhs_name = parse_var(to_Parse)->to_string();
    
    skip_whitespace(to_Parse);
    parse_keyword(to_Parse, "=");
    Expr* rhs = parse_expr(to_Parse);
    
    parse_keyword(to_Parse, "_in");
    skip_whitespace(to_Parse);
    Expr* body = parse_expr(to_Parse);
    
    return new LetExpr(lhs_name, rhs, body);
}

TEST_CASE("parse_let"){
    //No spaces
    {
        //the _ has been consumed at this point
        std::stringstream testing ("letx=5_inx+3");
        CHECK(parse_let(testing)->equals(new LetExpr("x", new NumExpr(5), new AddExpr(new VarExpr("x"), new NumExpr (3)))));
    }
    //some spaces
    {
        std::stringstream testing ("let  x  =   5   _in  x   +  3");
        CHECK(parse_let(testing)->equals(new LetExpr("x", new NumExpr(5), new AddExpr(new VarExpr("x"), new NumExpr (3)))));
    }
}

Expr* parse_if(std::istream &to_Parse){
    parse_keyword(to_Parse, "if");
    skip_whitespace(to_Parse);
    Expr* comparison = parse_expr(to_Parse);
    
    skip_whitespace(to_Parse);
    parse_keyword(to_Parse, "_then");
    Expr* if_true = parse_expr(to_Parse);
    
    parse_keyword(to_Parse, "_else");
    skip_whitespace(to_Parse);
    Expr* if_false = parse_expr(to_Parse);
    
    return new IfExpr(comparison, if_true, if_false);
}

TEST_CASE("parse_if"){
    //No spaces
    {
        //the _ has been consumed at this point
        std::stringstream testing ("ifx==5_then5_else6");
        CHECK(parse_if(testing)->equals(new IfExpr(new EqExpr(new VarExpr("x"), new NumExpr(5)), new NumExpr(5), new NumExpr(6))));
    }
    //some spaces
    {
        std::stringstream testing ("if  \n  x   \n   ==    \n   5  \n    _then \n    5   \n   _else  \n    6");
        CHECK(parse_if(testing)->equals(new IfExpr(new EqExpr(new VarExpr("x"), new NumExpr(5)), new NumExpr(5), new NumExpr(6))));
    }
}

Expr* parse_fun(std::istream &to_Parse){
    parse_keyword(to_Parse, "un");
    skip_whitespace(to_Parse);
    Expr* formal_arg = parse_expr(to_Parse);
    
    skip_whitespace(to_Parse);
    Expr* body = parse_expr(to_Parse);
    
    return new FunExpr(formal_arg->to_string(), body);
}

TEST_CASE("parse_fun"){
    
}

//Parsing Functions
Expr* parse_expr(std::istream & to_Parse){
    Expr* e = parse_comparg(to_Parse);
    
    skip_whitespace(to_Parse);
    
    int c = to_Parse.peek();
    if (c == '=') {
        consume_character(to_Parse, '=');
        c = to_Parse.peek();
        if (c == '=') {
            consume_character(to_Parse, '=');
            Expr* rhs = parse_expr(to_Parse);
            return new EqExpr(e, rhs);
        }
        throw std::runtime_error("Invalid Operand...");
    }
    return e;
}

TEST_CASE("parse expr"){
    {
        std::stringstream rep_cout (" 3 == 4");
        CHECK(parse_expr(rep_cout)->equals(new EqExpr(new NumExpr(3), new NumExpr(4))));
    }
    {
        std::stringstream rep_cout (" 3 =? 4");
        CHECK_THROWS_WITH(parse_expr(rep_cout), "Invalid Operand...");
    }
    {
        std::stringstream rep_cout (" 3 + 4");
        CHECK(parse_expr(rep_cout)->equals(new AddExpr(new NumExpr(3), new NumExpr(4))));
    }
    {
        std::stringstream rep_cout (" 3 * 4");
        CHECK(parse_expr(rep_cout)->equals(new MultExpr(new NumExpr(3), new NumExpr(4))));
    }
    {
        std::stringstream rep_cout (" 3 ");
        CHECK(parse_expr(rep_cout)->equals(new NumExpr(3)));
    }
    {
        std::stringstream rep_cout ("    _let     f     =    _fun        (x)   x+    1     _in   f    (   5   )   ");
        CHECK(parse_expr(rep_cout)->equals(new LetExpr("f", new FunExpr("x",new AddExpr(new VarExpr("x"), new NumExpr(1))), new CallExpr(new VarExpr("f"), new NumExpr(5)))));
    }
    {
        std::stringstream rep_cout ("_let factrl = _fun (factrl) _fun (x) _if x == 1 _then 1 _else x * factrl(factrl)(x + -1) _in  factrl(factrl)(10)");
        EqExpr* conditional = new EqExpr(new VarExpr("x"), new NumExpr(1));
        MultExpr* multElse = new MultExpr(new VarExpr("x"), new CallExpr(new CallExpr(new VarExpr("factrl"), new VarExpr("factrl")), new AddExpr(new VarExpr("x"), new NumExpr(-1))));
        IfExpr* insideIfElse = new IfExpr(conditional, new NumExpr(1), multElse);
        FunExpr* insidefunction = new FunExpr("x", insideIfElse);
        FunExpr* function_rhs = new FunExpr("factrl", insidefunction);
        LetExpr* overAll = new LetExpr("factrl", function_rhs, new CallExpr(new CallExpr(new VarExpr("factrl"), new VarExpr("factrl")), new NumExpr(10)));
        CHECK(parse_expr(rep_cout)->equals(overAll));
    }
}

Expr* parse_comparg(std::istream & to_Parse){
    Expr* e = parse_addend(to_Parse);
    
    skip_whitespace(to_Parse);
    
    int c = to_Parse.peek();
    if (c == '+') {
        consume_character(to_Parse, '+');
        Expr* rhs = parse_comparg(to_Parse);
        return new AddExpr(e, rhs);
    }
    else
        return e;
}

TEST_CASE("parse_comparg"){
    {
        std::stringstream rep_cout (" 3 + 4");
        CHECK(parse_comparg(rep_cout)->equals(new AddExpr(new NumExpr(3), new NumExpr(4))));
    }
    {
        std::stringstream rep_cout (" 3 * 4");
        CHECK(parse_comparg(rep_cout)->equals(new MultExpr(new NumExpr(3), new NumExpr(4))));
    }
    {
        std::stringstream rep_cout (" 3 ");
        CHECK(parse_comparg(rep_cout)->equals(new NumExpr(3)));
    }
    {
        //the == is ignored and just 3 is returned according the present lodgic we have
        std::stringstream rep_cout (" 3 == 4");
        CHECK(parse_comparg(rep_cout)->equals(new NumExpr(3)));
    }
}

Expr* parse_addend(std::istream &to_Parse){
    Expr* e = parse_multicand(to_Parse);
    
    skip_whitespace(to_Parse);
    
    int c = to_Parse.peek();
    if (c == '*') {
        consume_character(to_Parse, '*');
        Expr* rhs = parse_addend(to_Parse);
        return new MultExpr(e, rhs);
    }
    else
        return e;
}

TEST_CASE("parse addend"){
    {
        std::stringstream rep_cout (" 3 * 4");
        CHECK(parse_addend(rep_cout)->equals(new MultExpr(new NumExpr(3), new NumExpr(4))));
    }
    {
        std::stringstream rep_cout (" 3 + 4");
        CHECK(parse_addend(rep_cout)->equals(new NumExpr(3)));
    }
    {
        std::stringstream rep_cout (" 3 ");
        CHECK(parse_addend(rep_cout)->equals(new NumExpr(3)));
    }
    {
        //the == is ignored and just 3 is returned according the lodgic
        std::stringstream rep_cout (" 3 == 4");
        CHECK(parse_addend(rep_cout)->equals(new NumExpr(3)));
    }
}

Expr* parse_multicand(std::istream &to_Parse){
    Expr* e = parse_inner(to_Parse);
    skip_whitespace(to_Parse);
    while (to_Parse.peek() == '(') {
        consume_character(to_Parse, '(');
        Expr* actual_arg = parse_expr(to_Parse);
        consume_character(to_Parse, ')');
        skip_whitespace(to_Parse);
        e = new CallExpr(e, actual_arg);
    }
    return e;
}

TEST_CASE("parse multicand"){
    {
        std::stringstream rep_cout ("         factrl   (   factrl   )       (   10   )   ");
        CHECK(parse_multicand(rep_cout)->equals(new CallExpr(new CallExpr(new VarExpr("factrl"), new VarExpr("factrl")), new NumExpr(10))));
    }
}

Expr* parse_inner(std::istream &to_Parse){
    skip_whitespace(to_Parse);
    
    int c = to_Parse.peek();
    if ((c == '-') || isdigit(c))
        return parse_num(to_Parse);
    else if (c == '('){
        consume_character(to_Parse, '(');
        Expr* e = parse_expr(to_Parse);
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
            return new BoolExpr(true);
        }
        else if (c == 'f'){
            consume_character(to_Parse, 'f');
            c = to_Parse.peek();
            if (c == 'a') {
                parse_keyword(to_Parse, "alse");
                return new BoolExpr(false);
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
        CHECK(parse_inner(testing)->equals(new NumExpr(4)));
    }
    {
        std::stringstream testing ("_ifx==5_then5_else6");
        CHECK(parse_inner(testing)->equals(new IfExpr(new EqExpr(new VarExpr("x"), new NumExpr(5)), new NumExpr(5), new NumExpr(6))));
    }
    {
        //the _ has been consumed at this point
        std::stringstream testing ("_letx=5_inx+3");
        CHECK(parse_inner(testing)->equals(new LetExpr("x", new NumExpr(5), new AddExpr(new VarExpr("x"), new NumExpr (3)))));
    }
    {
        std::stringstream testing ("_true");
        CHECK(parse_inner(testing)->equals(new BoolExpr(true)));
    }
    {
        std::stringstream testing ("_false");
        CHECK(parse_inner(testing)->equals(new BoolExpr(false)));
    }
    
//    _let factrl = _fun (factrl)
//                    _fun (x)
//                      _if x == 1
//                      _then 1
//                      _else x * factrl(factrl)(x + -1)
//    _in  factrl(factrl)(10)
}





/*Num Implementation*/
//Default Constructor
NumExpr::NumExpr(int rep){
    this->rep = rep;
}

//Methods
bool NumExpr::equals(Expr *e){
    NumExpr *ne = dynamic_cast<NumExpr*>(e);
    if (ne == NULL)
        return false;
    else
        return this->rep == ne->rep;
}

Val* NumExpr::interp(){
    return new NumVal(this->rep);
}

bool NumExpr::has_variable(){
    return false;
}

Expr* NumExpr::subst(std::string s, Expr *e){
    return this;
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
        NumExpr* one = new NumExpr(1);
        NumExpr* two = new NumExpr(2);
        MultExpr* mult_one_two = new MultExpr(one,two);
        AddExpr* add_one_two = new AddExpr(one,two);

    //Checking Num Equality
        CHECK(one->equals(new NumExpr(1)));

    //Checking Num Inequality
        CHECK(!(one->equals(two)));

    //Checking Class Inequality
        CHECK(!(one->equals(mult_one_two)));
        CHECK(!(one->equals(add_one_two)));

    //Checking interp()
        CHECK(one->interp()->equals(new NumVal(1)));
        CHECK(two->interp()->equals(new NumVal(2)));

    //Checking has_variable()
        CHECK(one->has_variable() == false);
        CHECK(two->has_variable() == false);

    //Checking subst()
        CHECK(one->subst("s", two)->equals(new NumExpr(1)));
        CHECK(two->subst("s", one)->equals(new NumExpr(2)));
    
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
AddExpr::AddExpr(Expr *lhs, Expr *rhs) {
    this->lhs = lhs;
    this->rhs = rhs;
}

//Methods
bool AddExpr::equals(Expr *e){
    AddExpr *ae = dynamic_cast<AddExpr*>(e);
    if (ae == NULL)
        return false;
    else
        return (this->lhs->equals(ae->lhs) && this->rhs->equals(ae->rhs));
}

Val* AddExpr::interp(){
    return this->lhs->interp()->add_to(this->rhs->interp());
}

bool AddExpr::has_variable(){
    return this->lhs->has_variable() || this->rhs->has_variable();
}

Expr* AddExpr::subst(std::string s, Expr *e){
    return new AddExpr(this->lhs->subst(s, e), this->rhs->subst(s, e));
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
        NumExpr* one = new NumExpr(1);
        NumExpr* two = new NumExpr(2);
        
        AddExpr* one_two = new AddExpr(one,two);
        AddExpr* two_one = new AddExpr(two,one);
        AddExpr* add_two_add_exper = new AddExpr(one_two, two_one);
        AddExpr* vars = new AddExpr(new VarExpr("x"), new NumExpr(3));
        AddExpr* vars2 = new AddExpr(new NumExpr(8), new VarExpr("y"));
        AddExpr* vars3 = new AddExpr(one_two, new AddExpr(new VarExpr("z"), new NumExpr(7)));
        AddExpr* vars4 = new AddExpr(new AddExpr(new VarExpr("a"), new NumExpr(3)), two_one);
        
        
        MultExpr* mult_one_two = new MultExpr(one,two);
        MultExpr* mult_two_one = new MultExpr(two,one);
        AddExpr* add_two_mult_exper = new AddExpr(mult_one_two, mult_two_one);
    
    
    //Checking Add Equality
        CHECK(one_two->equals(new AddExpr(new NumExpr(1), new NumExpr(2))));
    
    //Checking Add Inequality
        CHECK(!(one_two->equals(two_one)));
    
    //Checking Class Inequality;
        CHECK(!(one_two->equals(one)));
        CHECK(!(one_two->equals(mult_one_two)));
    
    //Checking interp()
        CHECK(one_two->interp()->equals(new NumVal(3)));
        CHECK(add_two_add_exper->interp()->equals(new NumVal(6)));
        CHECK(add_two_mult_exper->interp()->equals(new NumVal(4)));
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
            CHECK((new AddExpr(new VarExpr("x"), new NumExpr(7)))->subst("x", new VarExpr("y"))->equals(new AddExpr(new VarExpr("y"), new NumExpr(7))));
            CHECK((new AddExpr(new VarExpr("x"), new NumExpr(7)))->subst("x", new NumExpr(3))->equals(new AddExpr(new NumExpr(3), new NumExpr(7))));
            CHECK((new AddExpr(new VarExpr("x"), new NumExpr(7)))->subst("x", new AddExpr(new VarExpr("x"), new NumExpr(8)))->equals(new AddExpr(new AddExpr(new VarExpr("x"), new NumExpr(8)), new NumExpr(7))));
            CHECK((new AddExpr(new VarExpr("x"), new NumExpr(7)))->subst("x", new MultExpr(new VarExpr("x"), new NumExpr(8)))->equals(new AddExpr(new MultExpr(new VarExpr("x"), new NumExpr(8)), new NumExpr(7))));
        //nested substitution
            CHECK((new AddExpr(new NumExpr(3), new AddExpr(new NumExpr(6), new VarExpr("y"))))->subst("y", new VarExpr("x"))->equals(new AddExpr(new NumExpr(3), new AddExpr(new NumExpr(6), new VarExpr("x")))));
        //no Var object present
            CHECK((new AddExpr(new NumExpr(8), new NumExpr(7)))->subst("x", new VarExpr("y"))->equals(new AddExpr(new NumExpr(8), new NumExpr(7))));
        //no matching string
            CHECK((new AddExpr(new VarExpr("x"), new NumExpr(7)))->subst("mismatch", new VarExpr("y"))->equals(new AddExpr(new VarExpr("x"), new NumExpr(7))));
    
    //Checking print()
        //No nesting
            {
                std::stringstream rep_cout ("");
                (new AddExpr(new NumExpr(1), new VarExpr("x")))->print(rep_cout);
                CHECK(rep_cout.str() == "(1+x)");
            }
        //Nested right
            {
                std::stringstream rep_cout("");
                (new AddExpr(new NumExpr(3),new AddExpr(new NumExpr(1), new VarExpr("x"))))->print(rep_cout);
                CHECK(rep_cout.str() == "(3+(1+x))");
            }
        //Nested left
            {
                std::stringstream rep_cout("");
                (new AddExpr(new AddExpr(new NumExpr(1), new VarExpr("x")), new NumExpr(3)))->print(rep_cout);
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
MultExpr::MultExpr(Expr *lhs, Expr *rhs) {
    this->lhs = lhs;
    this->rhs = rhs;
}

//Methods
bool MultExpr::equals(Expr *e){
    MultExpr* me = dynamic_cast<MultExpr*>(e);
    if (me == NULL)
        return false;
    else
        return (this->lhs->equals(me->lhs) && this->rhs->equals(me->rhs));
}

Val* MultExpr::interp(){
    return this->lhs->interp()->mult_by(this->rhs->interp());
}

bool MultExpr::has_variable(){
    return this->lhs->has_variable() || this->rhs->has_variable();
}

Expr* MultExpr::subst(std::string s, Expr *e){
    return new MultExpr(this->lhs->subst(s, e), this->rhs->subst(s, e));
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
        NumExpr* one = new NumExpr(1);
        NumExpr* two = new NumExpr(2);

        AddExpr* one_two = new AddExpr(one,two);
        AddExpr* two_one = new AddExpr(two,one);

        MultExpr* mult_one_two = new MultExpr(one,two);
        MultExpr* mult_two_one = new MultExpr(two,one);
        MultExpr* mult_two_mult_exper = new MultExpr(mult_one_two, mult_two_one);
        MultExpr* mult_two_add_exper = new MultExpr(one_two, two_one);
        MultExpr* vars = new MultExpr(new VarExpr("x"), new NumExpr(3));
        MultExpr* vars2 = new MultExpr(new NumExpr(8), new VarExpr("y"));
        MultExpr* vars3 = new MultExpr(one_two, new MultExpr(new VarExpr("z"), new NumExpr(7)));
        MultExpr* vars4 = new MultExpr(new MultExpr(new VarExpr("a"), new NumExpr(3)), two_one);
    
    
    //Checking Mult Equality
        CHECK(mult_one_two->equals(new MultExpr(new NumExpr(1), new NumExpr(2))));
    
    //Checking Mult Inequality
        CHECK(!(mult_one_two->equals(mult_two_one)));
    
    //Checking Class Inequality
        CHECK(!(mult_one_two->equals(one_two)));
        CHECK(!(mult_one_two->equals(one)));
    
    //Checking interp()
        CHECK(mult_one_two->interp()->equals(new NumVal(2)));
        CHECK(mult_two_add_exper->interp()->equals(new NumVal(9)));
        CHECK(mult_two_mult_exper->interp()->equals(new NumVal(4)));
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
            CHECK((new MultExpr(new VarExpr("x"), new NumExpr(7)))->subst("x", new VarExpr("y"))->equals(new MultExpr(new VarExpr("y"), new NumExpr(7))));
            CHECK((new MultExpr(new VarExpr("x"), new NumExpr(7)))->subst("x", new NumExpr(3))->equals(new MultExpr(new NumExpr(3), new NumExpr(7))));
            CHECK((new MultExpr(new VarExpr("x"), new NumExpr(7)))->subst("x", new AddExpr(new VarExpr("x"), new NumExpr(8)))->equals(new MultExpr(new AddExpr(new VarExpr("x"), new NumExpr(8)), new NumExpr(7))));
            CHECK((new MultExpr(new VarExpr("x"), new NumExpr(7)))->subst("x", new MultExpr(new VarExpr("x"), new NumExpr(8)))->equals(new MultExpr(new MultExpr(new VarExpr("x"), new NumExpr(8)), new NumExpr(7))));
        //nested substitution
            CHECK((new MultExpr(new NumExpr(3), new MultExpr(new NumExpr(6), new VarExpr("y"))))->subst("y", new VarExpr("x"))->equals(new MultExpr(new NumExpr(3), new MultExpr(new NumExpr(6), new VarExpr("x")))));
        //no Var object present
            CHECK((new MultExpr(new NumExpr(8), new NumExpr(7)))->subst("x", new VarExpr("y"))->equals(new MultExpr(new NumExpr(8), new NumExpr(7))));
        //no matching string
            CHECK((new MultExpr(new VarExpr("x"), new NumExpr(7)))->subst("mismatch", new VarExpr("y"))->equals(new MultExpr(new VarExpr("x"), new NumExpr(7))));
    
    //Checking print()
        //No nesting
            {
                std::stringstream rep_cout ("");
                (new MultExpr(new NumExpr(1), new VarExpr("x")))->print(rep_cout);
                CHECK(rep_cout.str() == "(1*x)");
            }
        //Nested right
            {
                std::stringstream rep_cout("");
                (new MultExpr(new NumExpr(3),new MultExpr(new NumExpr(1), new VarExpr("x"))))->print(rep_cout);
                CHECK(rep_cout.str() == "(3*(1*x))");
            }
        //Nested left
            {
                std::stringstream rep_cout("");
                (new MultExpr(new MultExpr(new NumExpr(1), new VarExpr("x")), new NumExpr(3)))->print(rep_cout);
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
bool VarExpr::equals(Expr *e){
    VarExpr *v = dynamic_cast<VarExpr*>(e);
    if(v == NULL)
        return false;
    else
        return this->var == v->var;
}

Val* VarExpr::interp(){
    throw std::runtime_error("The variable has no value assigned to it yet.");
}

bool VarExpr::has_variable(){
    return true;
}

Expr* VarExpr::subst(std::string s, Expr *e){
    if(this->var == s)
        return e;
    else
        return this;
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
        NumExpr* one = new NumExpr(1);
        NumExpr* two = new NumExpr(2);
        AddExpr* one_two = new AddExpr(one,two);
        MultExpr* mult_one_two = new MultExpr(one,two);
        VarExpr* first = new VarExpr("first");
        VarExpr* second = new VarExpr("second");

    //Checking Var Equality
        CHECK(first->equals(new VarExpr("first")));

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
        CHECK(first->subst("first", new AddExpr(new NumExpr(3), new VarExpr("x")))->equals(new AddExpr(new NumExpr(3), new VarExpr("x"))));
        CHECK(first->subst("second", new VarExpr("second"))->equals(new VarExpr("first")));
    
    //Checking print()
        {
            std::stringstream rep_cout ("");
            (new VarExpr("x"))->print(rep_cout);
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
LetExpr::LetExpr(std::string lhs_name, Expr *rhs,Expr *body){
    this->lhs_name = lhs_name;
    this->rhs = rhs;
    this->body = body;
}

//Methods
bool LetExpr::equals(Expr *e){
    LetExpr *l = dynamic_cast<LetExpr*>(e);
    if(l == NULL)
        return false;
    else
        return this->lhs_name == l->lhs_name && this->rhs->equals(l->rhs) && this->body->equals(l->body);
}

Val* LetExpr::interp(){
    return this->body->subst(this->lhs_name, rhs->interp()->to_expr())->interp();
}

bool LetExpr::has_variable(){
    return this->rhs->has_variable() || this->body->has_variable();
}

Expr* LetExpr::subst(std::string s, Expr *e){
    if(this->lhs_name != s){
        return new LetExpr(this->lhs_name, this->rhs->subst(s, e), this->body->subst(s, e));
    }
    return new LetExpr(this->lhs_name, this->rhs->subst(s, e), this->body);
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
    LetExpr* firstExpression = new LetExpr("x", new NumExpr(7), new MultExpr(new VarExpr("x"), new NumExpr(7)));
    LetExpr* secondExpression = new LetExpr("x", new NumExpr(7), new LetExpr("x", new NumExpr(8), new AddExpr(new VarExpr("x"), new NumExpr(7))));
    LetExpr* thridExpression = new LetExpr("x", new NumExpr(7), new LetExpr("x", new AddExpr(new VarExpr("x"), new NumExpr(8)), new AddExpr(new VarExpr("x"), new NumExpr(7))));
    LetExpr* fourthExpression = new LetExpr("x", new NumExpr(7), new LetExpr("y", new NumExpr(8), new AddExpr(new VarExpr("x"), new NumExpr(7))));
    LetExpr* invalidExpression_body_free_var = new LetExpr("x", new NumExpr(7), new LetExpr("x", new AddExpr(new VarExpr("x"), new NumExpr(8)), new AddExpr(new VarExpr("y"), new NumExpr(7))));
    LetExpr* invalidExpression_rhs_free_var = new LetExpr("y", new VarExpr("y"), new AddExpr(new NumExpr(3), new VarExpr("y")));
    LetExpr* noVariablePresent =new LetExpr("x", new NumExpr(7), new MultExpr(new NumExpr(5), new NumExpr(7)));
    
    //print variables
    LetExpr* print_test = new LetExpr("x", new NumExpr(5), new AddExpr(new LetExpr("y", new NumExpr(3), new AddExpr(new VarExpr("y"), new NumExpr(2))), new VarExpr("x")));
    
    //Checking _let Equality
    CHECK(firstExpression->equals(new LetExpr("x", new NumExpr(7), new MultExpr(new VarExpr("x"), new NumExpr(7)))));

    //Checking _let Inequality
    CHECK(!(firstExpression->equals(new LetExpr("y", new NumExpr(7), new MultExpr(new VarExpr("x"), new NumExpr(7))))));
    CHECK(!(firstExpression->equals(new LetExpr("x", new NumExpr(8), new MultExpr(new VarExpr("x"), new NumExpr(7))))));
    CHECK(!(firstExpression->equals(new LetExpr("x", new NumExpr(7), new MultExpr(new VarExpr("y"), new NumExpr(7))))));
    CHECK(!(firstExpression->equals(new LetExpr("x", new NumExpr(7), new MultExpr(new VarExpr("x"), new NumExpr(8))))));
    CHECK(!(firstExpression->equals(new LetExpr("x", new NumExpr(7), new MultExpr(new NumExpr(7), new VarExpr("x"))))));
    
    //Checking Class Inequality
    CHECK(!firstExpression->equals(new VarExpr("x")));
    CHECK(!firstExpression->equals(new NumExpr(7)));
    CHECK(!firstExpression->equals(new AddExpr(new NumExpr(0), new NumExpr(0))));
    CHECK(!firstExpression->equals(new MultExpr(new NumExpr(0), new NumExpr(0))));
    
    //Checking subst()
    // _let x = 1
    // _in  x + 2  -> subst("x", y+3)
    // ==
    // _let x = 1
    // _in  x + 2
    Expr* noSubstitutionNeeded = new LetExpr("x", new NumExpr(1),new AddExpr(new VarExpr("x"), new NumExpr(2)));
    CHECK(noSubstitutionNeeded->subst("x", new AddExpr(new VarExpr("y"), new NumExpr(3)))->equals(new LetExpr("x", new NumExpr(1),new AddExpr(new VarExpr("x"), new NumExpr(2)))));
    
    // _let z = x
    // _in  z + 2 -> subst("z", 0)
    // =
    // _let z = x
    // _in  z + 2
    Expr* noSubstitutionNeeded_DifferentVariable = new LetExpr("z", new VarExpr("x"), new AddExpr(new VarExpr("z"), new NumExpr(2)));
    CHECK(noSubstitutionNeeded_DifferentVariable->subst("z", new NumExpr(0))->equals(new LetExpr("z", new VarExpr("x"), new AddExpr(new VarExpr("z"), new NumExpr(2)))));
    
    // _let x = x
    // _in  x + 2  -> subst("x", y+3)
    // ==
    // _let x = y+3
    // _in  x + 2
    Expr* subOnlyTheRHS_SameVariable = new LetExpr("x", new VarExpr("x"), new AddExpr(new VarExpr("x"), new NumExpr(2)));
    CHECK(subOnlyTheRHS_SameVariable->subst("x", new AddExpr(new VarExpr("y"), new NumExpr(3)))->equals(new LetExpr("x", new AddExpr(new VarExpr("y"), new NumExpr(3)), new AddExpr(new VarExpr("x"), new NumExpr(2)))));
    
    // _let x = y
    // _in  x + 2 -> subst("y", 8)
    // =
    // _let x = 8
    // _in  x + 2
    Expr* subOnlyTheRHS_DifferentVariable = new LetExpr("x", new VarExpr("y"), new AddExpr(new VarExpr("x"), new NumExpr(2)));
    CHECK(subOnlyTheRHS_DifferentVariable->subst("y", new NumExpr(8))->equals(new LetExpr("x", new NumExpr(8), new AddExpr(new VarExpr("x"), new NumExpr(2)))));
    
    // _let z = z + 2
    // _in  z + 2 -> subst("z", 0)
    // =
    // _let z = 0 + 2
    // _in  z + 2
    Expr* subOnlyRHS_Addition = new LetExpr("z", new AddExpr(new VarExpr("z"), new NumExpr(2)), new AddExpr(new VarExpr("z"), new NumExpr(2)));
    CHECK(subOnlyRHS_Addition->subst("z", new NumExpr(0))->equals(new LetExpr("z", new AddExpr(new NumExpr(0), new NumExpr(2)), new AddExpr(new VarExpr("z"), new NumExpr(2)))));
    
    // _let x = y
    // _in  x + y -> subst("y", 8)
    // ==
    // _let x = 8
    // _in  x + 8
    Expr* subTheRHSAndBody = new LetExpr("x", new VarExpr("y"), new AddExpr(new VarExpr("x"), new VarExpr("y")));
    CHECK(subTheRHSAndBody->subst("y", new NumExpr(8))->equals(new LetExpr("x", new NumExpr(8), new AddExpr(new VarExpr("x"), new NumExpr(8)))));
    
    // _let x = 8
    // _in  x + 2 + y -> subst("y", 9)
    // =
    // _let x = 8
    // _in  x + 2 + 9
    Expr* subWithinBodyOnly = new LetExpr("x", new NumExpr(8), new AddExpr(new VarExpr("x"), new AddExpr(new NumExpr(2), new VarExpr("y"))));
    CHECK(subWithinBodyOnly->subst("y", new NumExpr(9))->equals(new LetExpr("x", new NumExpr(8), new AddExpr(new VarExpr("x"), new AddExpr(new NumExpr(2), new NumExpr(9))))));
    
    
    //Checking interp()
    CHECK(firstExpression->interp()->equals(new NumVal(49)));
    CHECK(secondExpression->interp()->equals(new NumVal(15)));
    CHECK(thridExpression->interp()->equals(new NumVal(22)));
    CHECK(fourthExpression->interp()->equals(new NumVal(14)));
    CHECK(noVariablePresent->interp()->equals(new NumVal(35)));
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
bool BoolExpr::equals(Expr *e){
    BoolExpr *be = dynamic_cast<BoolExpr*>(e);
    if(be == NULL)
        return false;
    else
        return this->rep == be->rep;
}

Val* BoolExpr::interp(){
    return new BoolVal(this->rep);
}

bool BoolExpr::has_variable(){
    return false;
}

Expr* BoolExpr::subst(std::string s, Expr *e){
    return this;
}

void BoolExpr::print(std::ostream& out){
    if (this->rep)
        out << "_true";
    else
        out << "_false";
}


TEST_CASE("BoolExpr Tests"){
    /* equals() */
    CHECK((new BoolExpr(false))->equals(new BoolExpr(false)));
    CHECK(!((new BoolExpr(false))->equals(new BoolExpr(true))));
    CHECK(!((new BoolExpr(false))->equals(new NumExpr(4))));
    
    /* interp() */
    CHECK((new BoolExpr(true))->interp()->equals(new BoolVal(true)));
    
    /* has_variable() */
    CHECK(!((new BoolExpr(true))->has_variable()));
    
    /* subst() */
    CHECK((new BoolExpr(true))->subst("s", new NumExpr(3))->equals(new BoolExpr(true)));
    
    /* print */
    {
        std::stringstream rep_cout ("");
        (new BoolExpr(true))->print(rep_cout);
        CHECK(rep_cout.str() == "_true");
    }
    {
        std::stringstream rep_cout ("");
        (new BoolExpr(false))->print(rep_cout);
        CHECK(rep_cout.str() == "_false");
    }
}





/* EqExpr Implementation */
//Default Constructor
EqExpr::EqExpr(Expr* lhs, Expr* rhs){
    this->lhs = lhs;
    this->rhs = rhs;
}

//Methods
bool EqExpr::equals(Expr *e){
    EqExpr* ee = dynamic_cast<EqExpr*>(e);
    if (ee == NULL)
        return false;
    else
        return (this->lhs->equals(ee->lhs) && this->rhs->equals(ee->rhs));
}

Val* EqExpr::interp(){
    if (this->lhs->interp()->equals(this->rhs->interp()))
        return new BoolVal(true);
    else
        return new BoolVal(false);
}

bool EqExpr::has_variable(){
    return this->lhs->has_variable() || this->rhs->has_variable();
}

Expr* EqExpr::subst(std::string s, Expr* e){
    return new EqExpr(this->lhs->subst(s, e), this->rhs->subst(s, e));
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
    CHECK((new EqExpr(new NumExpr(0), new NumExpr(1)))->equals(new EqExpr(new NumExpr(0), new NumExpr(1))));
    CHECK(!((new EqExpr(new NumExpr(0), new NumExpr(0)))->equals(new EqExpr(new NumExpr(0), new NumExpr(1)))));
    CHECK(!((new EqExpr(new NumExpr(1), new NumExpr(1)))->equals(new EqExpr(new NumExpr(0), new NumExpr(1)))));
    CHECK(!((new EqExpr(new NumExpr(1), new NumExpr(1)))->equals(new NumExpr(0))));
    
    /* interp() */
    CHECK((new EqExpr(new BoolExpr(false), new BoolExpr(false)))->interp()->equals(new BoolVal(true)));
    CHECK((new EqExpr(new BoolExpr(false), new BoolExpr(true)))->interp()->equals(new BoolVal(false)));
    CHECK((new EqExpr(new NumExpr(1), new BoolExpr(true)))->interp()->equals(new BoolVal(false)));
    CHECK((new EqExpr(new BoolExpr(false), new NumExpr(0)))->interp()->equals(new BoolVal(false)));
    
    
    /* has_variable() */
    CHECK((new EqExpr(new VarExpr("x"), new NumExpr(1)))->has_variable());
    CHECK((new EqExpr(new NumExpr(1), new VarExpr("x")))->has_variable());
    CHECK(!((new EqExpr(new NumExpr(1), new BoolExpr(true)))->has_variable()));
    
    /* subst() */
    CHECK((new EqExpr(new VarExpr("x"), new NumExpr(1)))->subst("x", new NumExpr(5))->equals(new EqExpr(new NumExpr(5), new NumExpr(1))));
    CHECK((new EqExpr(new NumExpr(1), new VarExpr("x")))->subst("x", new NumExpr(5))->equals(new EqExpr(new NumExpr(1), new NumExpr(5))));
    CHECK((new EqExpr(new NumExpr(1), new NumExpr(6)))->subst("x", new NumExpr(5))->equals(new EqExpr(new NumExpr(1), new NumExpr(6))));
    CHECK((new EqExpr(new NumExpr(1), new VarExpr("y")))->subst("x", new NumExpr(5))->equals(new EqExpr(new NumExpr(1), new VarExpr("y"))));
    CHECK((new EqExpr(new VarExpr("y"), new NumExpr(1)))->subst("x", new NumExpr(5))->equals(new EqExpr(new VarExpr("y"), new NumExpr(1))));
    
    /* print() */
    {
        std::stringstream rep_cout ("");
        (new EqExpr(new VarExpr("x"), new NumExpr(1)))->print(rep_cout);
        CHECK(rep_cout.str() == "(x==1)");
    }
    {
        std::stringstream rep_cout ("");
        (new EqExpr(new VarExpr("7"), new BoolExpr(false)))->print(rep_cout);
        CHECK(rep_cout.str() == "(7==_false)");
    }
}





/* IfExpr Implementation */
//Default Constructor
IfExpr::IfExpr(Expr* comparison, Expr* if_true, Expr* if_false){
    this->comparison = comparison;
    this->if_true = if_true;
    this->if_false = if_false;
}

//Methods
bool IfExpr::equals(Expr *e){
    IfExpr* ife = dynamic_cast<IfExpr*>(e);
    if (ife == NULL)
        return false;
    else
        return this->comparison->equals(ife->comparison) && this->if_true->equals(ife->if_true) && this->if_false->equals(ife->if_false);
}

Val* IfExpr::interp(){
    if(this->comparison->interp()->is_true())
        return this->if_true->interp();
    else
        return this->if_false->interp();
    
}

bool IfExpr::has_variable(){
    return this->comparison->has_variable() || this->if_true->has_variable() || this->if_false->has_variable();
}
Expr* IfExpr::subst(std::string s, Expr *e){
    return new IfExpr(comparison->subst(s, e), if_true->subst(s, e), if_false->subst(s, e));
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
    CHECK((new IfExpr(new NumExpr(0), new NumExpr(0) , new NumExpr(0)))->equals(new IfExpr(new NumExpr(0), new NumExpr(0) , new NumExpr(0))));
    CHECK(!((new IfExpr(new NumExpr(0), new NumExpr(0) , new NumExpr(0)))->equals(new NumExpr(0))));
    CHECK(!((new IfExpr(new NumExpr(1), new NumExpr(0) , new NumExpr(0)))->equals(new IfExpr(new NumExpr(0), new NumExpr(0) , new NumExpr(0)))));
    CHECK(!((new IfExpr(new NumExpr(0), new NumExpr(1) , new NumExpr(0)))->equals(new IfExpr(new NumExpr(0), new NumExpr(0) , new NumExpr(0)))));
    CHECK(!((new IfExpr(new NumExpr(0), new NumExpr(0) , new NumExpr(1)))->equals(new IfExpr(new NumExpr(0), new NumExpr(0) , new NumExpr(0)))));
    CHECK(!((new IfExpr(new VarExpr("x"), new NumExpr(0) , new NumExpr(0)))->equals(new IfExpr(new NumExpr(0), new NumExpr(0) , new NumExpr(0)))));
    CHECK(!((new IfExpr(new NumExpr(0), new VarExpr("x") , new NumExpr(0)))->equals(new IfExpr(new NumExpr(0), new NumExpr(0) , new NumExpr(0)))));
    CHECK(!((new IfExpr(new NumExpr(0), new NumExpr(0) , new VarExpr("x")))->equals(new IfExpr(new NumExpr(0), new NumExpr(0) , new NumExpr(0)))));
    
    /* interp() */
    CHECK((new IfExpr(new BoolExpr(true), new NumExpr(1), new NumExpr(0)))->interp()->equals(new NumVal(1)));
    CHECK((new IfExpr(new BoolExpr(false), new NumExpr(1), new NumExpr(0)))->interp()->equals(new NumVal(0)));
    
    /* has_variable() */
    CHECK(!((new IfExpr(new NumExpr(0), new NumExpr(0) , new NumExpr(0)))->has_variable()));
    CHECK((new IfExpr(new VarExpr("x"), new NumExpr(0) , new NumExpr(0)))->has_variable());
    CHECK((new IfExpr(new NumExpr(0), new VarExpr("x") , new NumExpr(0)))->has_variable());
    CHECK((new IfExpr(new NumExpr(0), new NumExpr(0) , new VarExpr("x")))->has_variable());
    
    /* subst() */
    CHECK((new IfExpr(new NumExpr(0), new NumExpr(0) , new NumExpr(0)))->subst("x", new NumExpr(5))->equals(new IfExpr(new NumExpr(0), new NumExpr(0) , new NumExpr(0))));
    CHECK((new IfExpr(new VarExpr("x"), new NumExpr(0) , new NumExpr(0)))->subst("x", new NumExpr(5))->equals(new IfExpr(new NumExpr(5), new NumExpr(0) , new NumExpr(0))));
    CHECK((new IfExpr(new NumExpr(0), new VarExpr("x") , new NumExpr(0)))->subst("x", new NumExpr(5))->equals(new IfExpr(new NumExpr(0), new NumExpr(5) , new NumExpr(0))));
    CHECK((new IfExpr(new NumExpr(0), new NumExpr(0) , new VarExpr("x")))->subst("x", new NumExpr(5))->equals(new IfExpr(new NumExpr(0), new NumExpr(0) , new NumExpr(5))));
    
    /* print() */
    {
        std::stringstream rep_cout ("");
        (new IfExpr(new NumExpr(2), new NumExpr(0) , new VarExpr("x")))->print(rep_cout);
        CHECK(rep_cout.str() == "(_if 2_then 0_else x)");
    }
    {
        std::stringstream rep_cout ("");
        (new IfExpr(new NumExpr(5), new NumExpr(6) , new NumExpr(4)))->print(rep_cout);
        CHECK(rep_cout.str() == "(_if 5_then 6_else 4)");
    }
}

/* FunExpr Implementations */
//Default Constructor
FunExpr::FunExpr(std::string formal_arg, Expr* body){
    this->formal_arg = formal_arg;
    this->body = body;
}

//Methods
bool FunExpr::equals(Expr* e){
    FunExpr* fe = dynamic_cast<FunExpr*>(e);
    if (fe == NULL)
        return false;
    else
        return this->formal_arg == fe->formal_arg && this->body->equals(fe->body);
}

Val* FunExpr::interp(){
    return new FunVal(this->formal_arg, this->body);
}

bool FunExpr::has_variable(){
    return false;
}

Expr* FunExpr::subst(std::string s, Expr* e){
    if(this->formal_arg != s){
        return new FunExpr(this->formal_arg, this->body->subst(s, e));
    }
    return this->body->subst(this->formal_arg, e);
}

void FunExpr::print(std::ostream& out){
    out << "(_fun (" + this->formal_arg + ") ";
    this->body->print(out);
    out << ")";
}

TEST_CASE("FunExpr Tests"){
    /* equals() */
    CHECK((new FunExpr("x", new AddExpr(new VarExpr("x"), new NumExpr(4))))->equals(new FunExpr("x", new AddExpr(new VarExpr("x"), new NumExpr(4)))));
    CHECK(!((new FunExpr("x", new AddExpr(new VarExpr("x"), new NumExpr(4))))->equals(new FunExpr("y", new AddExpr(new VarExpr("x"), new NumExpr(4))))));
    CHECK(!((new FunExpr("x", new AddExpr(new VarExpr("x"), new NumExpr(4))))->equals(new FunExpr("x", new AddExpr(new VarExpr("z"), new NumExpr(4))))));
    CHECK(!((new FunExpr("x", new AddExpr(new VarExpr("x"), new NumExpr(4))))->equals(new BoolExpr(true))));
    
    /* interp() */
    CHECK((new FunExpr("x", new AddExpr(new VarExpr("x"), new NumExpr(4))))->interp()->equals(new FunVal("x", new AddExpr(new VarExpr("x"), new NumExpr(4)))));
    
    /* has_variable() */
    CHECK(!((new FunExpr("x", new AddExpr(new VarExpr("x"), new NumExpr(4))))->has_variable()));
    
    /* subst() */
    CHECK((new FunExpr("x", new AddExpr(new VarExpr("x"), new VarExpr("y"))))->subst("x", new NumExpr(4))->equals(new AddExpr(new NumExpr(4), new VarExpr("y"))));
    CHECK((new FunExpr("x", new AddExpr(new VarExpr("x"), new VarExpr("y"))))->subst("y", new NumExpr(4))->equals(new FunExpr("x", new AddExpr(new VarExpr("x"), new NumExpr(4)))));
    
    
    /* print() */
    {
        std::stringstream rep_cout ("");
        (new FunExpr("x", new AddExpr(new VarExpr("x"), new NumExpr(4))))->print(rep_cout);
        CHECK(rep_cout.str() == "(_fun (x) (x+4))");
    }
}

/* CallExpr */
//Default Constructor
CallExpr::CallExpr(Expr* to_be_called, Expr* actual_arg){
    this->to_be_called = to_be_called;
    this->actual_arg = actual_arg;
}

//Methods
bool CallExpr::equals(Expr *e){
    CallExpr* ce = dynamic_cast<CallExpr*>(e);
    if (ce == NULL)
        return false;
    else
        return this->to_be_called->equals(ce->to_be_called) && this->actual_arg->equals(ce->actual_arg);
}

Val* CallExpr::interp(){
    return to_be_called->interp()->call(actual_arg->interp());
}

bool CallExpr::has_variable(){
    return false;
}

Expr* CallExpr::subst(std::string s, Expr *e){
    return new CallExpr(this->to_be_called->subst(s, e), this->actual_arg->subst(s, e));
}

void CallExpr::print(std::ostream& out){
    this->to_be_called->print(out);
    out << "(";
    this->actual_arg->print(out);
    out << ")";
}

TEST_CASE("CallExpr Tests"){
    /* equals() */
    CHECK((new CallExpr(new VarExpr("x"), new MultExpr(new VarExpr("x"), new NumExpr(4))))->equals(new CallExpr(new VarExpr("x"), new MultExpr(new VarExpr("x"), new NumExpr(4)))));
    CHECK(!((new CallExpr(new VarExpr("x"), new MultExpr(new VarExpr("x"), new NumExpr(4))))->equals(new CallExpr(new VarExpr("y"), new MultExpr(new VarExpr("x"), new NumExpr(4))))));
    CHECK(!((new CallExpr(new VarExpr("x"), new MultExpr(new VarExpr("x"), new NumExpr(4))))->equals(new CallExpr(new VarExpr("x"), new MultExpr(new VarExpr("y"), new NumExpr(4))))));
    CHECK(!((new CallExpr(new VarExpr("x"), new MultExpr(new VarExpr("x"), new NumExpr(4))))->equals(new CallExpr(new BoolExpr(true), new MultExpr(new VarExpr("x"), new NumExpr(4))))));
    CHECK(!((new CallExpr(new VarExpr("x"), new MultExpr(new VarExpr("x"), new NumExpr(4))))->equals(new BoolExpr(true))));
    
    /* interp() */
    CHECK((new CallExpr(new FunExpr("x", new MultExpr(new VarExpr("x"), new NumExpr(4))), new NumExpr(4)))->interp()->equals(new NumVal(16)));
    
    /* has_variable() */
    CHECK(!((new CallExpr(new FunExpr("x", new MultExpr(new VarExpr("x"), new NumExpr(4))), new NumExpr(4)))->has_variable()));
    
    /* subst() */
    CHECK((new CallExpr(new VarExpr("f"), new NumExpr(5)))->subst("f", new FunExpr("x", new AddExpr(new VarExpr("x"), new NumExpr(1))))->equals(new CallExpr(new FunExpr("x", new AddExpr(new VarExpr("x"), new NumExpr(1))), new NumExpr(5))));
    CHECK((new CallExpr(new VarExpr("x"), new NumExpr(5)))->subst("f", new FunExpr("x", new AddExpr(new VarExpr("x"), new NumExpr(1))))->equals(new CallExpr(new VarExpr("x"), new NumExpr(5))));
    CHECK((new CallExpr(new VarExpr("x"), new AddExpr(new NumExpr(4), new VarExpr("y"))))->subst("y", new NumExpr(4))->equals(new CallExpr(new VarExpr("x"), new AddExpr(new NumExpr(4),new NumExpr(4)))));
    FunExpr* functionReturnfunction = new FunExpr("x", new FunExpr("y", new AddExpr(new VarExpr("x"), new VarExpr("y"))));
    CHECK((new CallExpr(new CallExpr(new VarExpr("f"), new NumExpr(5)), new NumExpr(1)))->subst("f", functionReturnfunction)->equals(new CallExpr(new CallExpr(new FunExpr("x", new FunExpr("y", new AddExpr(new VarExpr("x"), new VarExpr("y")))), new NumExpr(5)), new NumExpr(1))));
    
    /* print() */
    {
        std::stringstream rep_cout ("");
        (new CallExpr(new VarExpr("f"), new NumExpr(4)))->print(rep_cout);
        CHECK(rep_cout.str() == "f(4)");
    }
    
}
