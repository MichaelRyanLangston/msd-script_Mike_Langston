//
//  expr.cpp
//  MSDscriptCommandLine
//
//  Created by Michael Langston on 1/24/21.
//

#include "expr.hpp"
#include "catch.h"

/*Num implementations*/
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

//Method Tests
TEST_CASE("Num_equals"){
    //Test Variables
    Num* one = new Num(1);
    Num* two = new Num(2);
    Mult * mult_one_two = new Mult(one,two);
    Add * add_one_two = new Add(one,two);
    //Checking Num Equality
    CHECK(one->equals(one));
    //Checking Num Inequality
    CHECK(!(one->equals(two)));
    //Checking Class Inequality
    CHECK(!(one->equals(mult_one_two)));
    CHECK(!(one->equals(add_one_two)));
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
        return (this->lhs == a->lhs) && (this->rhs == a->rhs);
}

//Method Tests
TEST_CASE("Add_equals"){
    //Test Varialbels
    Num* one = new Num(1);
    Num* two = new Num(2);
    Add * one_two = new Add(one,two);
    Add * two_one = new Add(two,one);
    Mult * mult_one_two = new Mult(one,two);
    //Checking Add Equality
    CHECK(one_two->equals(one_two));
    //Checking Add Inequality
    CHECK(!(one_two->equals(two_one)));
    //Checking Class Inequality;
    CHECK(!(one_two->equals(one)));
    CHECK(!(one_two->equals(mult_one_two)));
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
        return (this->lhs == m->lhs) && (this->rhs == m->rhs);
}

//Method Tests
TEST_CASE("Mult_equals"){
    //Test Varialbels
    Num* one = new Num(1);
    Num* two = new Num(2);
    Add * one_two = new Add(one,two);
    Mult * mult_one_two = new Mult(one,two);
    Mult * mult_two_one = new Mult(two,one);
    //Check Mult Equality
    CHECK(mult_one_two->equals(mult_one_two));
    //Check Mult Inequality
    CHECK(!(mult_one_two->equals(mult_two_one)));
    //Check Class Inequality
    CHECK(!(mult_one_two->equals(one_two)));
    CHECK(!(mult_one_two->equals(one)));
}

/*Var implementations*/
//Default Constructor
Var::Var(std::string val){
    this->val = val;
}

//Methods
bool Var::equals(Expr *e){
    Var *v = dynamic_cast<Var*>(e);
    if(v == NULL)
        return false;
    else
        return this->val == v->val;
}

//Method Tests
TEST_CASE("Var_equals"){
    //Test Varialbels
    Num* one = new Num(1);
    Num* two = new Num(2);
    Add * one_two = new Add(one,two);
    Mult * mult_one_two = new Mult(one,two);
    Var * first = new Var("first");
    Var * second = new Var("second");
    //Check Mult Equality
    CHECK(first->equals(first));
    //Check Mult Inequality
    CHECK(!(first->equals(second)));
    //Check Class Inequality
    CHECK(!(first->equals(one)));
    CHECK(!(first->equals(one_two)));
    CHECK(!(first->equals(mult_one_two)));
}
