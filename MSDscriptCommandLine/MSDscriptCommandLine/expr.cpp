//
//  expr.cpp
//  MSDscriptCommandLine
//
//  Created by Michael Langston on 1/24/21.
//

#include "expr.hpp"
#include "catch.h"
#include <stdexcept>

//stringstream libaray
#include <sstream>
#include <iostream>



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
    CHECK((new Mult(new Add(new Num(0), new Var("y")), new Num(1)))->to_string() == "((0+y)*1)");
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
    
    return new Num(to_Return);
}

TEST_CASE("parse_num"){
    {
        std::stringstream testing ("33");
        CHECK(parse_num(testing)->equals(new Num(33)));
    }
    
    {
        std::stringstream testing ("-33");
        CHECK(parse_num(testing)->equals(new Num(-33)));
    }
    
    {
        std::stringstream testing ("-");
        CHECK(parse_num(testing)->equals(new Num(0)));
    }
    
    {
        std::stringstream testing ("g");
        CHECK(parse_num(testing)->equals(new Num(0)));
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
    return new Var(to_Return);
}

TEST_CASE("parse_var"){
    {
        std::stringstream testing ("g");
        CHECK(parse_var(testing)->equals(new Var("g")));
    }
    
    {
        std::stringstream testing ("guess0");
        CHECK(parse_var(testing)->equals(new Var("guess")));
    }
    
    {
        std::stringstream testing ("gu0ess");
        CHECK(parse_var(testing)->equals(new Var("gu")));
    }
    
    {
        std::stringstream testing ("0guess");
        CHECK(parse_var(testing)->equals(new Var("")));
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
    
    return new _let(lhs_name, rhs, body);
}

TEST_CASE("parse_let"){
    {
        std::stringstream testing ("_let x=5 _in  x + 3");
        CHECK(parse_let(testing)->equals(new _let("x", new Num(5), new Add(new Var("x"), new Num (3)))));
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
        return new Add(e, rhs);
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
        return new Mult(e, rhs);
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
Num::Num(int val){
    this->val = val;
}

//Methods
bool Num::equals(Expr *e){
    Num *n = dynamic_cast<Num*>(e);
    if (n == NULL)
        return false;
    else
        return this->val == n->val;
}

int Num::interp(){
    return this->val;
}

bool Num::has_variable(){
    return false;
}

Expr* Num::subst(std::string s, Expr *e){
    return this;
}

void Num::print(std::ostream& out){
    out << this->val;
}

/*Depricated*/
//void Num::pretty_print_at(std::ostream& out, print_mode_t mode, int num){
//    print(out);
//}

//Method Tests
TEST_CASE("Num"){
    
    //Test Variables
        Num* one = new Num(1);
        Num* two = new Num(2);
        Mult* mult_one_two = new Mult(one,two);
        Add* add_one_two = new Add(one,two);

    //Checking Num Equality
        CHECK(one->equals(new Num(1)));

    //Checking Num Inequality
        CHECK(!(one->equals(two)));

    //Checking Class Inequality
        CHECK(!(one->equals(mult_one_two)));
        CHECK(!(one->equals(add_one_two)));

    //Checking interp()
        CHECK(one->interp() == 1);
        CHECK(two->interp() == 2);

    //Checking has_variable()
        CHECK(one->has_variable() == false);
        CHECK(two->has_variable() == false);

    //Checking subs()
        CHECK(one->subst("s", two)->equals(new Num(1)));
        CHECK(two->subst("s", one)->equals(new Num(2)));
    
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
Add::Add(Expr *lhs, Expr *rhs) {
    this->lhs = lhs;
    this->rhs = rhs;
}

//Methods
bool Add::equals(Expr *e){
    Add *a = dynamic_cast<Add*>(e);
    if (a == NULL)
        return false;
    else
        return (this->lhs->equals(a->lhs) && this->rhs->equals(a->rhs));
}

int Add::interp(){
    return this->lhs->interp() + this->rhs->interp();
}

bool Add::has_variable(){
    return this->lhs->has_variable() || this->rhs->has_variable();
}

Expr* Add::subst(std::string s, Expr *e){
    return new Add(this->lhs->subst(s, e), this->rhs->subst(s, e));
}

void Add::print(std::ostream& out){
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
        Num* one = new Num(1);
        Num* two = new Num(2);
        
        Add* one_two = new Add(one,two);
        Add* two_one = new Add(two,one);
        Add* add_two_add_exper = new Add(one_two, two_one);
        Add* vars = new Add(new Var("x"), new Num(3));
        Add* vars2 = new Add(new Num(8), new Var("y"));
        Add* vars3 = new Add(one_two, new Add(new Var("z"), new Num(7)));
        Add* vars4 = new Add(new Add(new Var("a"), new Num(3)), two_one);
        
        
        Mult* mult_one_two = new Mult(one,two);
        Mult* mult_two_one = new Mult(two,one);
        Add* add_two_mult_exper = new Add(mult_one_two, mult_two_one);
    
    
    //Checking Add Equality
        CHECK(one_two->equals(new Add(new Num(1), new Num(2))));
    
    //Checking Add Inequality
        CHECK(!(one_two->equals(two_one)));
    
    //Checking Class Inequality;
        CHECK(!(one_two->equals(one)));
        CHECK(!(one_two->equals(mult_one_two)));
    
    //Checking interp()
        CHECK(one_two->interp() == 3);
        CHECK(add_two_add_exper->interp() == 6);
        CHECK(add_two_mult_exper->interp() == 4);
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
            CHECK((new Add(new Var("x"), new Num(7)))->subst("x", new Var("y"))->equals(new Add(new Var("y"), new Num(7))));
            CHECK((new Add(new Var("x"), new Num(7)))->subst("x", new Num(3))->equals(new Add(new Num(3), new Num(7))));
            CHECK((new Add(new Var("x"), new Num(7)))->subst("x", new Add(new Var("x"), new Num(8)))->equals(new Add(new Add(new Var("x"), new Num(8)), new Num(7))));
            CHECK((new Add(new Var("x"), new Num(7)))->subst("x", new Mult(new Var("x"), new Num(8)))->equals(new Add(new Mult(new Var("x"), new Num(8)), new Num(7))));
        //nested substitution
            CHECK((new Add(new Num(3), new Add(new Num(6), new Var("y"))))->subst("y", new Var("x"))->equals(new Add(new Num(3), new Add(new Num(6), new Var("x")))));
        //no Var object present
            CHECK((new Add(new Num(8), new Num(7)))->subst("x", new Var("y"))->equals(new Add(new Num(8), new Num(7))));
        //no matching string
            CHECK((new Add(new Var("x"), new Num(7)))->subst("mismatch", new Var("y"))->equals(new Add(new Var("x"), new Num(7))));
    
    //Checking print()
        //No nesting
            {
                std::stringstream rep_cout ("");
                (new Add(new Num(1), new Var("x")))->print(rep_cout);
                CHECK(rep_cout.str() == "(1+x)");
            }
        //Nested right
            {
                std::stringstream rep_cout("");
                (new Add(new Num(3),new Add(new Num(1), new Var("x"))))->print(rep_cout);
                CHECK(rep_cout.str() == "(3+(1+x))");
            }
        //Nested left
            {
                std::stringstream rep_cout("");
                (new Add(new Add(new Num(1), new Var("x")), new Num(3)))->print(rep_cout);
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
Mult::Mult(Expr *lhs, Expr *rhs) {
    this->lhs = lhs;
    this->rhs = rhs;
}

//Methods
bool Mult::equals(Expr *e){
    Mult *m = dynamic_cast<Mult*>(e);
    if (m == NULL)
        return false;
    else
        return (this->lhs->equals(m->lhs) && this->rhs->equals(m->rhs));
}

int Mult::interp(){
    return this->lhs->interp() * this->rhs->interp();
}

bool Mult::has_variable(){
    return this->lhs->has_variable() || this->rhs->has_variable();
}

Expr* Mult::subst(std::string s, Expr *e){
    return new Mult(this->lhs->subst(s, e), this->rhs->subst(s, e));
}

void Mult::print(std::ostream& out){
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
        Num* one = new Num(1);
        Num* two = new Num(2);

        Add* one_two = new Add(one,two);
        Add* two_one = new Add(two,one);

        Mult* mult_one_two = new Mult(one,two);
        Mult* mult_two_one = new Mult(two,one);
        Mult* mult_two_mult_exper = new Mult(mult_one_two, mult_two_one);
        Mult* mult_two_add_exper = new Mult(one_two, two_one);
        Mult* vars = new Mult(new Var("x"), new Num(3));
        Mult* vars2 = new Mult(new Num(8), new Var("y"));
        Mult* vars3 = new Mult(one_two, new Mult(new Var("z"), new Num(7)));
        Mult* vars4 = new Mult(new Mult(new Var("a"), new Num(3)), two_one);
    
    
    //Checking Mult Equality
        CHECK(mult_one_two->equals(new Mult(new Num(1), new Num(2))));
    
    //Checking Mult Inequality
        CHECK(!(mult_one_two->equals(mult_two_one)));
    
    //Checking Class Inequality
        CHECK(!(mult_one_two->equals(one_two)));
        CHECK(!(mult_one_two->equals(one)));
    
    //Checking interp()
        CHECK(mult_one_two->interp() == 2);
        CHECK(mult_two_add_exper->interp() == 9);
        CHECK(mult_two_mult_exper->interp() == 4);
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
            CHECK((new Mult(new Var("x"), new Num(7)))->subst("x", new Var("y"))->equals(new Mult(new Var("y"), new Num(7))));
            CHECK((new Mult(new Var("x"), new Num(7)))->subst("x", new Num(3))->equals(new Mult(new Num(3), new Num(7))));
            CHECK((new Mult(new Var("x"), new Num(7)))->subst("x", new Add(new Var("x"), new Num(8)))->equals(new Mult(new Add(new Var("x"), new Num(8)), new Num(7))));
            CHECK((new Mult(new Var("x"), new Num(7)))->subst("x", new Mult(new Var("x"), new Num(8)))->equals(new Mult(new Mult(new Var("x"), new Num(8)), new Num(7))));
        //nested substitution
            CHECK((new Mult(new Num(3), new Mult(new Num(6), new Var("y"))))->subst("y", new Var("x"))->equals(new Mult(new Num(3), new Mult(new Num(6), new Var("x")))));
        //no Var object present
            CHECK((new Mult(new Num(8), new Num(7)))->subst("x", new Var("y"))->equals(new Mult(new Num(8), new Num(7))));
        //no matching string
            CHECK((new Mult(new Var("x"), new Num(7)))->subst("mismatch", new Var("y"))->equals(new Mult(new Var("x"), new Num(7))));
    
    //Checking print()
        //No nesting
            {
                std::stringstream rep_cout ("");
                (new Mult(new Num(1), new Var("x")))->print(rep_cout);
                CHECK(rep_cout.str() == "(1*x)");
            }
        //Nested right
            {
                std::stringstream rep_cout("");
                (new Mult(new Num(3),new Mult(new Num(1), new Var("x"))))->print(rep_cout);
                CHECK(rep_cout.str() == "(3*(1*x))");
            }
        //Nested left
            {
                std::stringstream rep_cout("");
                (new Mult(new Mult(new Num(1), new Var("x")), new Num(3)))->print(rep_cout);
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
Var::Var(std::string val){
    this->var = val;
}

//Methods
bool Var::equals(Expr *e){
    Var *v = dynamic_cast<Var*>(e);
    if(v == NULL)
        return false;
    else
        return this->var == v->var;
}

int Var::interp(){
    throw std::runtime_error("The variable has no value assigned to it yet.");
}

bool Var::has_variable(){
    return true;
}

Expr* Var::subst(std::string s, Expr *e){
    if(this->var == s)
        return e;
    else
        return this;
}

void Var::print(std::ostream& out){
    out << this->var;
}


/*Depricated*/
//void Var::pretty_print_at(std::ostream& out, print_mode_t mode, int num){
//    print(out);
//}

//Method Tests
TEST_CASE("Var"){

    //Test Varialbels
        Num* one = new Num(1);
        Num* two = new Num(2);
        Add* one_two = new Add(one,two);
        Mult* mult_one_two = new Mult(one,two);
        Var* first = new Var("first");
        Var* second = new Var("second");

    //Checking Var Equality
        CHECK(first->equals(new Var("first")));

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
        CHECK(first->subst("first", new Add(new Num(3), new Var("x")))->equals(new Add(new Num(3), new Var("x"))));
        CHECK(first->subst("second", new Var("second"))->equals(new Var("first")));
    
    //Checking print()
        {
            std::stringstream rep_cout ("");
            (new Var("x"))->print(rep_cout);
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
_let::_let(std::string lhs_name, Expr *rhs,Expr *body){
    this->lhs_name = lhs_name;
    this->rhs = rhs;
    this->body = body;
}

//Methods
bool _let::equals(Expr *e){
    _let *l = dynamic_cast<_let*>(e);
    if(l == NULL)
        return false;
    else
        return this->lhs_name == l->lhs_name && this->rhs->equals(l->rhs) && this->body->equals(l->body);
}

int _let::interp(){
    return this->body->subst(this->lhs_name, new Num(this->rhs->interp()))->interp();
}

bool _let::has_variable(){
    return this->rhs->has_variable() || this->body->has_variable();
}

Expr* _let::subst(std::string s, Expr *e){
    if(this->lhs_name == s){
        this->rhs = this->rhs->subst(s, e);
    }
    else{
        this->rhs = this->rhs->subst(s, e);
        this->body = this->body->subst(s, e);
    }
    return this;
}

void _let::print(std::ostream& out){
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
    _let* firstExpression = new _let("x", new Num(7), new Mult(new Var("x"), new Num(7)));
    _let* secondExpression = new _let("x", new Num(7), new _let("x", new Num(8), new Add(new Var("x"), new Num(7))));
    _let* thridExpression = new _let("x", new Num(7), new _let("x", new Add(new Var("x"), new Num(8)), new Add(new Var("x"), new Num(7))));
    _let* fourthExpression = new _let("x", new Num(7), new _let("y", new Num(8), new Add(new Var("x"), new Num(7))));
    _let* invalidExpression_body_free_var = new _let("x", new Num(7), new _let("x", new Add(new Var("x"), new Num(8)), new Add(new Var("y"), new Num(7))));
    _let* invalidExpression_rhs_free_var = new _let("y", new Var("y"), new Add(new Num(3), new Var("y")));
    _let* noVariablePresent =new _let("x", new Num(7), new Mult(new Num(5), new Num(7)));
    
    //print variables
    _let* print_test = new _let("x", new Num(5), new Add(new _let("y", new Num(3), new Add(new Var("y"), new Num(2))), new Var("x")));
    
    //Checking _let Equality
    CHECK(firstExpression->equals(new _let("x", new Num(7), new Mult(new Var("x"), new Num(7)))));

    //Checking _let Inequality
    CHECK(!(firstExpression->equals(new _let("y", new Num(7), new Mult(new Var("x"), new Num(7))))));
    CHECK(!(firstExpression->equals(new _let("x", new Num(8), new Mult(new Var("x"), new Num(7))))));
    CHECK(!(firstExpression->equals(new _let("x", new Num(7), new Mult(new Var("y"), new Num(7))))));
    CHECK(!(firstExpression->equals(new _let("x", new Num(7), new Mult(new Var("x"), new Num(8))))));
    CHECK(!(firstExpression->equals(new _let("x", new Num(7), new Mult(new Num(7), new Var("x"))))));
    
    //Checking Class Inequality
    CHECK(!firstExpression->equals(new Var("x")));
    CHECK(!firstExpression->equals(new Num(7)));
    CHECK(!firstExpression->equals(new Add(new Num(0), new Num(0))));
    CHECK(!firstExpression->equals(new Mult(new Num(0), new Num(0))));
    
    //Checking subst()
    // _let x = 1
    // _in  x + 2  -> subst("x", y+3)
    // ==
    // _let x = 1
    // _in  x + 2
    Expr* noSubstitutionNeeded = new _let("x", new Num(1),new Add(new Var("x"), new Num(2)));
    CHECK(noSubstitutionNeeded->subst("x", new Add(new Var("y"), new Num(3)))->equals(new _let("x", new Num(1),new Add(new Var("x"), new Num(2)))));
    
    // _let z = x
    // _in  z + 2 -> subst("z", 0)
    // =
    // _let z = x
    // _in  z + 2
    Expr* noSubstitutionNeeded_DifferentVariable = new _let("z", new Var("x"), new Add(new Var("z"), new Num(2)));
    CHECK(noSubstitutionNeeded_DifferentVariable->subst("z", new Num(0))->equals(new _let("z", new Var("x"), new Add(new Var("z"), new Num(2)))));
    
    // _let x = x
    // _in  x + 2  -> subst("x", y+3)
    // ==
    // _let x = y+3
    // _in  x + 2
    Expr* subOnlyTheRHS_SameVariable = new _let("x", new Var("x"), new Add(new Var("x"), new Num(2)));
    CHECK(subOnlyTheRHS_SameVariable->subst("x", new Add(new Var("y"), new Num(3)))->equals(new _let("x", new Add(new Var("y"), new Num(3)), new Add(new Var("x"), new Num(2)))));
    
    // _let x = y
    // _in  x + 2 -> subst("y", 8)
    // =
    // _let x = 8
    // _in  x + 2
    Expr* subOnlyTheRHS_DifferentVariable = new _let("x", new Var("y"), new Add(new Var("x"), new Num(2)));
    CHECK(subOnlyTheRHS_DifferentVariable->subst("y", new Num(8))->equals(new _let("x", new Num(8), new Add(new Var("x"), new Num(2)))));
    
    // _let z = z + 2
    // _in  z + 2 -> subst("z", 0)
    // =
    // _let z = 0 + 2
    // _in  z + 2
    Expr* subOnlyRHS_Addition = new _let("z", new Add(new Var("z"), new Num(2)), new Add(new Var("z"), new Num(2)));
    CHECK(subOnlyRHS_Addition->subst("z", new Num(0))->equals(new _let("z", new Add(new Num(0), new Num(2)), new Add(new Var("z"), new Num(2)))));
    
    // _let x = y
    // _in  x + y -> subst("y", 8)
    // ==
    // _let x = 8
    // _in  x + 8
    Expr* subTheRHSAndBody = new _let("x", new Var("y"), new Add(new Var("x"), new Var("y")));
    CHECK(subTheRHSAndBody->subst("y", new Num(8))->equals(new _let("x", new Num(8), new Add(new Var("x"), new Num(8)))));
    
    // _let x = 8
    // _in  x + 2 + y -> subst("y", 9)
    // =
    // _let x = 8
    // _in  x + 2 + 9
    Expr* subWithinBodyOnly = new _let("x", new Num(8), new Add(new Var("x"), new Add(new Num(2), new Var("y"))));
    CHECK(subWithinBodyOnly->subst("y", new Num(9))->equals(new _let("x", new Num(8), new Add(new Var("x"), new Add(new Num(2), new Num(9))))));
    
    
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
//    Add* exampleTests = new Add(new Mult(new Num(5), new _let("x", new Num(5), new Var("x"))), new Num(1));
//    {
//        std::stringstream rep_cout ("");
//        print_test->pretty_print(rep_cout);
//        CHECK(rep_cout.str() == "_let x = 5\n_in  (_let y = 3\n      _in  y + 2) + x");
//    }
}
