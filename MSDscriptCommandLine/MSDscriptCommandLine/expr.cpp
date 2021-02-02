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


/*Exper Implemntation Interface*/
//Methods
std::string Expr::to_string(){
    std::stringstream stringify("");
    this->print(stringify);
    return stringify.str();
}

void Expr::pretty_print(std::ostream& out){
    pretty_print_at(out, print_group_none);
}

//Method Tests
TEST_CASE("Exper Implemntation"){
    CHECK((new Mult(new Add(new Num(0), new Var("y")), new Num(1)))->to_string() == "((0+y)*1)");
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

void Num::pretty_print_at(std::ostream& out, print_mode_t mode){
    print(out);
}

//Method Tests
TEST_CASE("Num Implementation"){
    
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
    
    //Checking pretty_print()
        {
            std::stringstream rep_cout ("");
            one->pretty_print(rep_cout);
            CHECK(rep_cout.str() == "1");
        }
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

void Add::pretty_print_at(std::ostream& out, print_mode_t mode){
    //lhs, rhs
    if(mode == print_group_none){
        this->lhs->pretty_print_at(out, print_group_add);
        out << " + ";
        this->rhs->pretty_print_at(out, print_group_none);
    }
    else if(mode == print_group_add){
        out << "(";
        this->lhs->pretty_print_at(out, print_group_add);
        out << " + ";
        this->rhs->pretty_print_at(out, print_group_none);
        out << ")";
    }
    else {
        out << "(";
        this->lhs->pretty_print_at(out, print_group_add);
        out << " + ";
        this->rhs->pretty_print_at(out, print_group_none);
        out << ")";
    }
}

//Method Tests
TEST_CASE("Add Test Cases"){
    
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
    //Checking pretty_print()
        //No nesting
            {
                std::stringstream rep_cout ("");
                (new Add(new Num(1), new Var("x")))->pretty_print(rep_cout);
                CHECK(rep_cout.str() == "1 + x");
            }
        //Add w/ Nested right Add
            {
                std::stringstream rep_cout("");
                (new Add(new Num(3),new Add(new Num(1), new Var("x"))))->pretty_print(rep_cout);
                CHECK(rep_cout.str() == "3 + 1 + x");
            }
        //Add w/ Nested left Add
            {
                std::stringstream rep_cout("");
                (new Add(new Add(new Num(1), new Var("x")), new Num(3)))->pretty_print(rep_cout);
                CHECK(rep_cout.str() == "(1 + x) + 3");
            }
        //Add w/ Nested right Mult
            {
                std::stringstream rep_cout("");
                (new Add(new Num(3), new Mult(new Num(1), new Var("x"))))->pretty_print(rep_cout);
                CHECK(rep_cout.str() == "3 + 1 * x");
            }
        //Add w/ Nested left Mult
            {
                std::stringstream rep_cout("");
                (new Add(new Mult(new Num(1), new Var("x")), new Num(3)))->pretty_print(rep_cout);
                CHECK(rep_cout.str() == "1 * x + 3");
            }
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

void Mult::pretty_print_at(std::ostream& out, print_mode_t mode){
    //lhs, rhs
    if(mode == print_group_none){
        this->lhs->pretty_print_at(out, print_group_add_or_mult);
        out << " * ";
        this->rhs->pretty_print_at(out, print_group_none);
    }
    else if(mode == print_group_add){
        this->lhs->pretty_print_at(out, print_group_add_or_mult);
        out << " * ";
        this->rhs->pretty_print_at(out, print_group_add_or_mult);
    }
    else {
        out << "(";
        this->lhs->pretty_print_at(out, print_group_add_or_mult);
        out << " * ";
        this->rhs->pretty_print_at(out, print_group_none);
        out << ")";
    }
}

//Method Tests
TEST_CASE("Mult_equals"){
    
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
    
    //Checking pretty_print()
        //No nesting
            {
                std::stringstream rep_cout ("");
                (new Mult(new Num(1), new Var("x")))->pretty_print(rep_cout);
                CHECK(rep_cout.str() == "1 * x");
            }
        //Mult w/ Nested right Add
            {
                std::stringstream rep_cout("");
                (new Mult(new Num(3),new Add(new Num(1), new Var("x"))))->pretty_print(rep_cout);
                CHECK(rep_cout.str() == "3 * (1 + x)");
            }
        //Mult w/ Nested left Add
            {
                std::stringstream rep_cout("");
                (new Mult(new Add(new Num(1), new Var("x")), new Num(3)))->pretty_print(rep_cout);
                CHECK(rep_cout.str() == "(1 + x) * 3");
            }
        //Mult w/ Nested right Mult
            {
                std::stringstream rep_cout("");
                (new Mult(new Num(3), new Mult(new Num(1), new Var("x"))))->pretty_print(rep_cout);
                CHECK(rep_cout.str() == "3 * 1 * x");
            }
        //Mult w/ Nested left Mult
            {
                std::stringstream rep_cout("");
                (new Mult(new Mult(new Num(1), new Var("x")), new Num(3)))->pretty_print(rep_cout);
                CHECK(rep_cout.str() == "(1 * x) * 3");
            }
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

void Var::pretty_print_at(std::ostream& out, print_mode_t mode){
    print(out);
}

//Method Tests
TEST_CASE("Var_equals"){

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
    //Checking pretty_print()
        {
            std::stringstream rep_cout ("");
            (new Var("x"))->pretty_print(rep_cout);
            CHECK(rep_cout.str() == "x");
        }
}
