//
//  expr.cpp
//  MSDscriptCommandLine
//
//  Created by Michael Langston on 1/24/21.
//

#include <stdexcept>
#include <sstream>
#include <iostream>
#include "expr.hpp"
#include "catch.h"
#include "val.hpp"


/*Exper Implemntation Interface*/
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
    CHECK((new MultExpr(new AddExpr(new NumExpr(new NumVal(0)), new VarExpr("y")), new NumExpr(new NumVal(0))))->to_string() == "((0+y)*1)");
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
        throw std::runtime_error("parse_num: invalid character...");
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
    
    return new NumExpr(new NumVal(to_Return));
}

TEST_CASE("parse_num"){
    {
        std::stringstream testing ("33");
        CHECK(parse_num(testing)->equals(new NumExpr(new NumVal(33))));
    }
    
    {
        std::stringstream testing ("-33");
        CHECK(parse_num(testing)->equals(new NumExpr(new NumVal(33))));
    }
    
    {
        std::stringstream testing ("--33");
        CHECK_THROWS_WITH(parse_num(testing), "parse_num: invalid character...");
    }
    
    {
        std::stringstream testing ("-");
        CHECK_THROWS_WITH(parse_num(testing), "parse_num: invalid character...");
    }
    
    {
        std::stringstream testing ("g");
        CHECK_THROWS_WITH(parse_num(testing), "parse_num: invalid character...");
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
    parse_keyword(to_Parse, "_let");
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
        std::stringstream testing ("_letx=5_inx+3");
        CHECK(parse_let(testing)->equals(new LetExpr("x", new NumExpr(new NumVal(5)), new AddExpr(new VarExpr("x"), new NumExpr(new NumVal(3))))));
    }
    //some spaces
    {
        std::stringstream testing ("_let  x  =   5   _in  x   +  3");
        CHECK(parse_let(testing)->equals(new LetExpr("x", new NumExpr(new NumVal(5)), new AddExpr(new VarExpr("x"), new NumExpr(new NumVal(3))))));
    }
}


//Parsing Functions
Expr* parse_expr(std::istream & to_Parse){
    Expr* e = parse_addend(to_Parse);
    
    skip_whitespace(to_Parse);
    
    int c = to_Parse.peek();
    if (c == '+') {
        consume_character(to_Parse, '+');
        Expr* rhs = parse_expr(to_Parse);
        return new AddExpr(e, rhs);
    }
    else
        return e;
}

TEST_CASE("parse_expr"){
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
}

Expr* parse_multicand(std::istream &to_Parse){
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
        return parse_let(to_Parse);
    }
    else{
        consume_character(to_Parse, c);
        throw std::runtime_error("invalid input");
    }
}

TEST_CASE("parse_multiccand"){
}






/*Num Implementation*/
//Default Constructor
NumExpr::NumExpr(int rep){
    this->rep = rep;
}

//Methods
bool NumExpr::equals(Expr* e){
    NumExpr* ne = dynamic_cast<NumExpr*>(e);
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
        NumExpr* one = new NumExpr(new NumVal(1));
        NumExpr* two = new NumExpr(new NumVal(2));
        MultExpr* mult_one_two = new MultExpr(one,two);
        AddExpr* add_one_two = new AddExpr(one,two);

    //Checking Num Equality
        CHECK(one->equals(new NumExpr(new NumVal(1))));

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

    //Checking subs()
        CHECK(one->subst("s", two)->equals(new NumExpr(new NumVal(1))));
        CHECK(two->subst("s", one)->equals(new NumExpr(new NumVal(1))));
    
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
    AddExpr* ae = dynamic_cast<AddExpr*>(e);
    if (ae == NULL)
        return false;
    else
        return (this->lhs->equals(ae->lhs) && this->rhs->equals(ae->rhs));
}

Val* AddExpr::interp(){
    return new NumVal(this->lhs->interp()->getVal() + this->rhs->interp()->getVal());
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
        NumExpr* one = new NumExpr(new NumVal(1));
        NumExpr* two = new NumExpr(new NumVal(2));
        
        AddExpr* one_two = new AddExpr(one,two);
        AddExpr* two_one = new AddExpr(two,one);
        AddExpr* add_two_add_exper = new AddExpr(one_two, two_one);
        AddExpr* vars = new AddExpr(new VarExpr("x"), new NumExpr(new NumVal(3)));
        AddExpr* vars2 = new AddExpr(new NumExpr(new NumVal(8)), new VarExpr("y"));
        AddExpr* vars3 = new AddExpr(one_two, new AddExpr(new VarExpr("z"), new NumExpr(new NumVal(7))));
        AddExpr* vars4 = new AddExpr(new AddExpr(new VarExpr("a"), new NumExpr(new NumVal(3))), two_one);
        
        
        MultExpr* mult_one_two = new MultExpr(one,two);
        MultExpr* mult_two_one = new MultExpr(two,one);
        AddExpr* add_two_mult_exper = new AddExpr(mult_one_two, mult_two_one);
    
    
    //Checking Add Equality
        CHECK(one_two->equals(new AddExpr(new NumExpr(new NumVal(1)), new NumExpr(new NumVal(2)))));
    
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
            CHECK((new AddExpr(new VarExpr("x"), new NumExpr(new NumVal(7))))->subst("x", new VarExpr("y"))->equals(new AddExpr(new VarExpr("y"), new NumExpr(new NumVal(7)))));
            CHECK((new AddExpr(new VarExpr("x"), new NumExpr(new NumVal(7))))->subst("x", new NumExpr(new NumVal(3)))->equals(new AddExpr(new NumExpr(new NumVal(3)), new NumExpr(new NumVal(7)))));
            CHECK((new AddExpr(new VarExpr("x"), new NumExpr(new NumVal(7))))->subst("x", new AddExpr(new VarExpr("x"), new NumExpr(new NumVal(8))))->equals(new AddExpr(new AddExpr(new VarExpr("x"), new NumExpr(new NumVal(8))), new NumExpr(new NumVal(7)))));
            CHECK((new AddExpr(new VarExpr("x"), new NumExpr(new NumVal(7))))->subst("x", new MultExpr(new VarExpr("x"), new NumExpr(new NumVal(8))))->equals(new AddExpr(new MultExpr(new VarExpr("x"), new NumExpr(new NumVal(8))), new NumExpr(new NumVal(7)))));
        //nested substitution
            CHECK((new AddExpr(new NumExpr(new NumVal(3)), new AddExpr(new NumExpr(new NumVal(6)), new VarExpr("y"))))->subst("y", new VarExpr("x"))->equals(new AddExpr(new NumExpr(new NumVal(3)), new AddExpr(new NumExpr(new NumVal(6)), new VarExpr("x")))));
        //no Var object present
            CHECK((new AddExpr(new NumExpr(new NumVal(8)), new NumExpr(new NumVal(7))))->subst("x", new VarExpr("y"))->equals(new AddExpr(new NumExpr(new NumVal(8)), new NumExpr(new NumVal(7)))));
        //no matching string
            CHECK((new AddExpr(new VarExpr("x"), new NumExpr(new NumVal(7))))->subst("mismatch", new VarExpr("y"))->equals(new AddExpr(new VarExpr("x"), new NumExpr(new NumVal(7)))));
    
    //Checking print()
        //No nesting
            {
                std::stringstream rep_cout ("");
                (new AddExpr(new NumExpr(new NumVal(1)), new VarExpr("x")))->print(rep_cout);
                CHECK(rep_cout.str() == "(1+x)");
            }
        //Nested right
            {
                std::stringstream rep_cout("");
                (new AddExpr(new NumExpr(new NumVal(3)),new AddExpr(new NumExpr(new NumVal(1)), new VarExpr("x"))))->print(rep_cout);
                CHECK(rep_cout.str() == "(3+(1+x))");
            }
        //Nested left
            {
                std::stringstream rep_cout("");
                (new AddExpr(new AddExpr(new NumExpr(new NumVal(1)), new VarExpr("x")), new NumExpr(new NumVal(3))))->print(rep_cout);
                CHECK(rep_cout.str() == "((1+x)+3)");
            }
    /*Depricated*/
//    //Checking pretty_print()
//        //No nesting
//            {
//                std::stringstream rep_cout ("");
//                (new Add(new NumExpr(1), new VarExpr("x")))->pretty_print(rep_cout);
//                CHECK(rep_cout.str() == "1 + x");
//            }
//        //Add w/ Nested right Add
//            {
//                std::stringstream rep_cout("");
//                (new Add(new NumExpr(3),new Add(new NumExpr(1), new VarExpr("x"))))->pretty_print(rep_cout);
//                CHECK(rep_cout.str() == "3 + 1 + x");
//            }
//        //Add w/ Nested left Add
//            {
//                std::stringstream rep_cout("");
//                (new Add(new Add(new NumExpr(1), new VarExpr("x")), new NumExpr(3)))->pretty_print(rep_cout);
//                CHECK(rep_cout.str() == "(1 + x) + 3");
//            }
//        //Add w/ Nested right Mult
//            {
//                std::stringstream rep_cout("");
//                (new Add(new NumExpr(3), new MultExpr(new NumExpr(1), new VarExpr("x"))))->pretty_print(rep_cout);
//                CHECK(rep_cout.str() == "3 + 1 * x");
//            }
//        //Add w/ Nested left Mult
//            {
//                std::stringstream rep_cout("");
//                (new Add(new MultExpr(new NumExpr(1), new VarExpr("x")), new NumExpr(3)))->pretty_print(rep_cout);
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
    MultExpr *m = dynamic_cast<MultExpr*>(e);
    if (m == NULL)
        return false;
    else
        return (this->lhs->equals(m->lhs) && this->rhs->equals(m->rhs));
}

Val* MultExpr::interp(){
    
    return new NumVal(this->lhs->interp()->getVal() * this->rhs->interp()->getVal());
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
//void MultExpr::pretty_print_at(std::ostream& out, print_mode_t mode, int num){
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
TEST_CASE("Mul"){
    
    //Test Varialbels
        NumExpr* one = new NumExpr(new NumVal(1));
        NumExpr* two = new NumExpr(new NumVal(2));

        AddExpr* one_two = new AddExpr(one,two);
        AddExpr* two_one = new AddExpr(two,one);

        MultExpr* mult_one_two = new MultExpr(one,two);
        MultExpr* mult_two_one = new MultExpr(two,one);
        MultExpr* mult_two_mult_exper = new MultExpr(mult_one_two, mult_two_one);
        MultExpr* mult_two_add_exper = new MultExpr(one_two, two_one);
        MultExpr* vars = new MultExpr(new VarExpr("x"), new NumExpr(new NumVal(3)));
        MultExpr* vars2 = new MultExpr(new NumExpr(new NumVal(8)), new VarExpr("y"));
        MultExpr* vars3 = new MultExpr(one_two, new MultExpr(new VarExpr("z"), new NumExpr(new NumVal(7))));
        MultExpr* vars4 = new MultExpr(new MultExpr(new VarExpr("a"), new NumExpr(new NumVal(3))), two_one);
    
    
    //Checking Mult Equality
        CHECK(mult_one_two->equals(new MultExpr(new NumExpr(new NumVal(1)), new NumExpr(new NumVal(2)))));
    
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
//                (new MultExpr(new NumExpr(1), new VarExpr("x")))->pretty_print(rep_cout);
//                CHECK(rep_cout.str() == "1 * x");
//            }
//        //Mult w/ Nested right Add
//            {
//                std::stringstream rep_cout("");
//                (new MultExpr(new NumExprExpr(3),new Add(new NumExpr(1), new VarExpr("x"))))->pretty_print(rep_cout);
//                CHECK(rep_cout.str() == "3 * (1 + x)");
//            }
//        //Mult w/ Nested left Add
//            {
//                std::stringstream rep_cout("");
//                (new MultExpr(new Add(new NumExpr(1), new VarExpr("x")), new NumExpr(3)))->pretty_print(rep_cout);
//                CHECK(rep_cout.str() == "(1 + x) * 3");
//            }
//        //Mult w/ Nested right MultExpr
//            {
//                std::stringstream rep_cout("");
//                (new MultExpr(new NumExpr(3), new MultExpr(new NumExpr(1), new VarExpr("x"))))->pretty_print(rep_cout);
//                CHECK(rep_cout.str() == "3 * 1 * x");
//            }
//        //MultExpr w/ Nested left MultExpr
//            {
//                std::stringstream rep_cout("");
//                (new MultExpr(new MultExpr(new NumExpr(1), new VarExpr("x")), new NumExpr(3)))->pretty_print(rep_cout);
//                CHECK(rep_cout.str() == "(1 * x) * 3");
//            }
//        //MultExpr w/ Nested left MultExpr
//            {
//                std::stringstream rep_cout("");
//                (new MultExpr(new MultExpr(new NumExpr(1), new VarExpr("x")), new Add(new VarExpr("y"), new NumExpr(5))))->pretty_print(rep_cout);
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

/*_let implementations*/
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
    return this->body->subst(this->lhs_name, new NumExpr(this->rhs->interp()))->interp();
}

bool LetExpr::has_variable(){
    return this->rhs->has_variable() || this->body->has_variable();
}

Expr* LetExpr::subst(std::string s, Expr *e){
    if(this->lhs_name == s){
        this->rhs = this->rhs->subst(s, e);
    }
    else{
        this->rhs = this->rhs->subst(s, e);
        this->body = this->body->subst(s, e);
    }
    return this;
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
    LetExpr* firstExpression = new _let("x", new NumExpr(7), new MultExpr(new VarExpr("x"), new NumExpr(7)));
    LetExpr* secondExpression = new _let("x", new NumExpr(7), new _let("x", new NumExpr(8), new AddExpr(new VarExpr("x"), new NumExpr(7))));
    LetExpr* thridExpression = new _let("x", new NumExpr(7), new _let("x", new AddExpr(new VarExpr("x"), new NumExpr(8)), new AddExpr(new VarExpr("x"), new NumExpr(7))));
    LetExpr* fourthExpression = new _let("x", new NumExpr(7), new _let("y", new NumExpr(8), new AddExpr(new VarExpr("x"), new NumExpr(7))));
    LetExpr* invalidExpression_body_free_VarExpr = new _let("x", new NumExpr(7), new _let("x", new AddExpr(new VarExpr("x"), new NumExpr(8)), new AddExpr(new VarExpr("y"), new NumExpr(7))));
    LetExpr* invalidExpression_rhs_free_var = new _let("y", new VarExpr("y"), new AddExpr(new NumExpr(3), new VarExpr("y")));
    LetExpr* noVariablePresent =new _let("x", new NumExpr(7), new MultExpr(new NumExpr(5), new NumExpr(7)));
    
    //print variables
    LetExpr* print_test = new _let("x", new NumExpr(5), new AddExpr(new _let("y", new NumExpr(3), new AddExpr(new VarExpr("y"), new NumExpr(2))), new VarExpr("x")));
    
    //Checking _let Equality
    CHECK(firstExpression->equals(new _let("x", new NumExpr(7), new MultExpr(new VarExpr("x"), new NumExpr(7)))));

    //Checking _let Inequality
    CHECK(!(firstExpression->equals(new _let("y", new NumExpr(7), new MultExpr(new VarExpr("x"), new NumExpr(7))))));
    CHECK(!(firstExpression->equals(new _let("x", new NumExpr(8), new MultExpr(new VarExpr("x"), new NumExpr(7))))));
    CHECK(!(firstExpression->equals(new _let("x", new NumExpr(7), new MultExpr(new VarExpr("y"), new NumExpr(7))))));
    CHECK(!(firstExpression->equals(new _let("x", new NumExpr(7), new MultExpr(new VarExpr("x"), new NumExpr(8))))));
    CHECK(!(firstExpression->equals(new _let("x", new NumExpr(7), new MultExpr(new NumExpr(7), new VarExpr("x"))))));
    
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
    Expr* noSubstitutionNeeded = new _let("x", new NumExpr(1),new AddExpr(new VarExpr("x"), new NumExpr(2)));
    CHECK(noSubstitutionNeeded->subst("x", new AddExpr(new VarExpr("y"), new NumExpr(3)))->equals(new _let("x", new NumExpr(1),new AddExpr(new VarExpr("x"), new NumExpr(2)))));
    
    // _let z = x
    // _in  z + 2 -> subst("z", 0)
    // =
    // _let z = x
    // _in  z + 2
    Expr* noSubstitutionNeeded_DifferentVariable = new _let("z", new VarExpr("x"), new AddExpr(new VarExpr("z"), new NumExpr(2)));
    CHECK(noSubstitutionNeeded_DifferentVariable->subst("z", new NumExpr(0))->equals(new _let("z", new VarExpr("x"), new AddExpr(new VarExpr("z"), new NumExpr(2)))));
    
    // _let x = x
    // _in  x + 2  -> subst("x", y+3)
    // ==
    // _let x = y+3
    // _in  x + 2
    Expr* subOnlyTheRHS_SameVariable = new _let("x", new VarExpr("x"), new AddExpr(new VarExpr("x"), new NumExpr(2)));
    CHECK(subOnlyTheRHS_SameVariable->subst("x", new AddExpr(new VarExpr("y"), new NumExpr(3)))->equals(new _let("x", new AddExpr(new VarExpr("y"), new NumExpr(3)), new AddExpr(new VarExpr("x"), new NumExpr(2)))));
    
    // _let x = y
    // _in  x + 2 -> subst("y", 8)
    // =
    // _let x = 8
    // _in  x + 2
    Expr* subOnlyTheRHS_DifferentVariable = new _let("x", new VarExpr("y"), new AddExpr(new VarExpr("x"), new NumExpr(2)));
    CHECK(subOnlyTheRHS_DifferentVariable->subst("y", new NumExpr(8))->equals(new _let("x", new NumExpr(8), new AddExpr(new VarExpr("x"), new NumExpr(2)))));
    
    // _let z = z + 2
    // _in  z + 2 -> subst("z", 0)
    // =
    // _let z = 0 + 2
    // _in  z + 2
    Expr* subOnlyRHS_AddExprition = new _let("z", new AddExpr(new VarExpr("z"), new NumExpr(2)), new AddExpr(new VarExpr("z"), new NumExpr(2)));
    CHECK(subOnlyRHS_AddExprition->subst("z", new NumExpr(0))->equals(new _let("z", new AddExpr(new NumExpr(0), new NumExpr(2)), new AddExpr(new VarExpr("z"), new NumExpr(2)))));
    
    // _let x = y
    // _in  x + y -> subst("y", 8)
    // ==
    // _let x = 8
    // _in  x + 8
    Expr* subTheRHSAndBody = new _let("x", new VarExpr("y"), new AddExpr(new VarExpr("x"), new VarExpr("y")));
    CHECK(subTheRHSAndBody->subst("y", new NumExpr(8))->equals(new _let("x", new NumExpr(8), new AddExpr(new VarExpr("x"), new NumExpr(8)))));
    
    // _let x = 8
    // _in  x + 2 + y -> subst("y", 9)
    // =
    // _let x = 8
    // _in  x + 2 + 9
    Expr* subWithinBodyOnly = new _let("x", new NumExpr(8), new AddExpr(new VarExpr("x"), new AddExpr(new NumExpr(2), new VarExpr("y"))));
    CHECK(subWithinBodyOnly->subst("y", new NumExpr(9))->equals(new _let("x", new NumExpr(8), new AddExpr(new VarExpr("x"), new AddExpr(new NumExpr(2), new NumExpr(9))))));
    
    
    //Checking interp()
    CHECK(firstExpression->interp() == 49);
    CHECK(secondExpression->interp() == 15);
    CHECK(thridExpression->interp() == 22);
    CHECK(fourthExpression->interp() == 14);
    CHECK(noVariablePresent->interp() == 35);
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
//    Add* exampleTests = new Add(new MultExpr(new NumExpr(5), new _let("x", new NumExpr(5), new Var("x"))), new NumExpr(1));
//    {
//        std::stringstream rep_cout ("");
//        print_test->pretty_print(rep_cout);
//        CHECK(rep_cout.str() == "_let x = 5\n_in  (_let y = 3\n      _in  y + 2) + x");
//    }
}
