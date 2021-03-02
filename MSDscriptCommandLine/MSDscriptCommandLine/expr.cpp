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
        CHECK(parse_let(testing)->equals(new LetExpr("x", new NumExpr(5), new AddExpr(new VarExpr("x"), new NumExpr (3)))));
    }
    //some spaces
    {
        std::stringstream testing ("_let  x  =   5   _in  x   +  3");
        CHECK(parse_let(testing)->equals(new LetExpr("x", new NumExpr(5), new AddExpr(new VarExpr("x"), new NumExpr (3)))));
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
    //Parse Normal Addition Function
    {
        std::stringstream testing ("3 + 4");
        CHECK(parse_expr(testing)->equals(new AddExpr(new NumExpr(3), new NumExpr(4))));
    }
    //Parse something that isn't an add function
    {
        std::stringstream testing ("3 * 4");
        CHECK(parse_let(testing)->equals(new MultExpr(new NumExpr(3), new NumExpr(4))));
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
    //Parse Normal Mult Function
    {
        std::stringstream testing ("3 * 4");
        CHECK(parse_let(testing)->equals(new MultExpr(new NumExpr(3), new NumExpr(4))));
    }
    //Parse something that isn't an Mult function
    {
        std::stringstream testing ("3 + 4");
        CHECK(parse_expr(testing)->equals(new AddExpr(new NumExpr(3), new NumExpr(4))));
    }
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
TEST_CASE("Mul"){
    
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
