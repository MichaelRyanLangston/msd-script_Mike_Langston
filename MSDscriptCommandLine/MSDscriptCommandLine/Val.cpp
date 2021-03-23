//
//  val.cpp
//  MSDscriptCommandLine
//
//  Created by Michael Langston on 3/1/21.
//

#include "val.hpp"
#include "expr.hpp"
#include "catch.h"
#include <sstream>

/* NumVal Implementation */

//Default Constructor
NumVal::NumVal(int rep){
    this->rep = rep;
}


//Methods
bool NumVal::equals(PTR(Val) v){
    PTR(NumVal) nv = CAST(NumVal)(v);
    if (nv == NULL)
        return false;
    else
        return this->rep == nv->rep;
}

PTR(Expr) NumVal::to_expr(){
    return NEW(NumExpr)(this->rep);
}

PTR(Val) NumVal::add_to(PTR(Val) other_val){
    PTR(NumVal) other_num = CAST(NumVal)(other_val);
    if(other_num == NULL)
        throw std::runtime_error("Non-NumVal object detected. Cannot perform addition.");
    return NEW(NumVal)(this->rep + other_num->rep);
}

PTR(Val) NumVal::mult_by(PTR(Val) other_val){
    PTR(NumVal) other_num = CAST(NumVal)(other_val);
    if(other_num == NULL)
        throw std::runtime_error("Non-NumVal object detected. Cannot perform multiplication.");
    return NEW(NumVal)(this->rep * other_num->rep);
}

bool NumVal::is_true(){
    throw std::runtime_error("Non-BoolVal object detected.");
}

PTR(Val) NumVal::call(PTR(Val) actual_arg){
    throw std::runtime_error("Non-FunVal object detected.");
}


//Testing
TEST_CASE("NumVal Tests"){
    
    /* equals() */
    CHECK((NEW(NumVal)(0))->equals(NEW(NumVal)(0)));
    CHECK(!((NEW(NumVal)(1))->equals(NEW(NumVal)(2))));
    CHECK(!((NEW(NumVal)(2))->equals(NEW(BoolVal)(true))));
    
    /* to_expr() */
    CHECK((NEW(NumVal)(3))->to_expr()->equals(NEW(NumExpr)(3)));
    
    /* add_to() */
    CHECK((NEW(NumVal)(4))->add_to(NEW(NumVal)(1))->equals(NEW(NumVal)(5)));
    CHECK_THROWS_WITH((NEW(NumVal)(4))->add_to(NEW(BoolVal)(true)), "Non-NumVal object detected. Cannot perform addition.");
    
    /* mult_by() */
    CHECK((NEW(NumVal)(4))->mult_by(NEW(NumVal)(1))->equals(NEW(NumVal)(4)));
    CHECK_THROWS_WITH((NEW(NumVal)(4))->mult_by(NEW(BoolVal)(true)), "Non-NumVal object detected. Cannot perform multiplication.");
    
    /* is_true() */
    CHECK_THROWS_WITH((NEW(NumVal)(4))->is_true(), "Non-BoolVal object detected.");
    
    /* call() */
    CHECK_THROWS_WITH((NEW(NumVal)(4))->call(NEW(NumVal)(4)), "Non-FunVal object detected.");
}





/* BoolVal Implentation */
//Default Constructor
BoolVal::BoolVal(bool rep){
    this->rep = rep;
}

//Methods
bool BoolVal::equals(PTR(Val) v){
    PTR(BoolVal) bv = CAST(BoolVal)(v);
    if (bv == NULL)
        return false;
    else
        return this->rep == bv->rep;
}

PTR(Expr) BoolVal::to_expr(){
    return NEW(BoolExpr)(this->rep);
}

PTR(Val) BoolVal::add_to(PTR(Val) other_val){
    throw std::runtime_error("Non-NumVal object detected. Cannot perform addition.");
}

PTR(Val) BoolVal::mult_by(PTR(Val) other_val){
    throw std::runtime_error("Non-NumVal object detected. Cannot perform multiplication.");
}

bool BoolVal::is_true(){
    return this->rep;
}

PTR(Val) BoolVal::call(PTR(Val) actual_arg){
    throw std::runtime_error("Non-FunVal object detected.");
}

TEST_CASE("BoolVal Tests"){
    /* equals() */
    CHECK((NEW(BoolVal)(false))->equals(NEW(BoolVal)(false)));
    CHECK(!((NEW(BoolVal)(true))->equals(NEW(BoolVal)(false))));
    CHECK(!((NEW(BoolVal)(true))->equals(NEW(NumVal)(0))));
    
    /* to_expr()*/
    CHECK((NEW(BoolVal)(true))->to_expr()->equals(NEW(BoolExpr)(true)));
    CHECK((NEW(BoolVal)(false))->to_expr()->equals(NEW(BoolExpr)(false)));
    
    /* add_to() */
    CHECK_THROWS_WITH((NEW(BoolVal)(true))->add_to(NEW(NumVal)(false)), "Non-NumVal object detected. Cannot perform addition.");
    
    /* mult_by() */
    CHECK_THROWS_WITH((NEW(BoolVal)(true))->mult_by(NEW(NumVal)(false)),"Non-NumVal object detected. Cannot perform multiplication.");
    
    /* is_true() */
    CHECK((NEW(BoolVal)(true))->is_true());
    CHECK(!((NEW(BoolVal)(false))->is_true()));
    
    /* call() */
    CHECK_THROWS_WITH((NEW(BoolVal)(true))->call(NEW(BoolVal)(true)), "Non-FunVal object detected.");
}





/* FunVal Implementations */
//Default Constructor
FunVal::FunVal(std::string formal_arg, PTR(Expr) body){
    this->formal_arg = formal_arg;
    this->body = body;
}

//Methods
bool FunVal::equals(PTR(Val) v){
    PTR(FunVal) fv = CAST(FunVal)(v);
    if (fv == NULL)
        return false;
    else
        return this->formal_arg == fv->formal_arg && this->body->equals(fv->body);
}

PTR(Expr) FunVal::to_expr(){
    return NEW(FunExpr)(this->formal_arg, this->body);
}

PTR(Val) FunVal::add_to(PTR(Val) other_val){
    throw std::runtime_error("Non-NumVal object detected. Cannot perform addition.");
}

PTR(Val) FunVal::mult_by(PTR(Val) other_val){
    throw std::runtime_error("Non-NumVal object detected. Cannot perform multiplication.");
}

bool FunVal::is_true(){
    throw std::runtime_error("Non-BoolVal object detected.");
}

PTR(Val) FunVal::call(PTR(Val) actual_arg){
    return this->body->subst(this->formal_arg, actual_arg->to_expr())->interp();
}

TEST_CASE("FunVal Tests"){
    /* equals() */
    CHECK((NEW(FunVal)("x", NEW(AddExpr)(NEW(VarExpr)("x"), NEW(NumExpr)(4))))->equals(NEW(FunVal)("x", NEW(AddExpr)(NEW(VarExpr)("x"), NEW(NumExpr)(4)))));
    CHECK(!((NEW(FunVal)("x", NEW(AddExpr)(NEW(VarExpr)("x"), NEW(NumExpr)(4))))->equals(NEW(FunVal)("y", NEW(AddExpr)(NEW(VarExpr)("x"), NEW(NumExpr)(4))))));
    CHECK(!((NEW(FunVal)("x", NEW(AddExpr)(NEW(VarExpr)("x"), NEW(NumExpr)(4))))->equals(NEW(FunVal)("x", NEW(MultExpr)(NEW(VarExpr)("x"), NEW(NumExpr)(4))))));
    CHECK(!((NEW(FunVal)("x", NEW(AddExpr)(NEW(VarExpr)("x"), NEW(NumExpr)(4))))->equals(NEW(BoolVal)(true))));
    /* to_expr()*/
    CHECK((NEW(FunVal)("x", NEW(AddExpr)(NEW(VarExpr)("x"), NEW(NumExpr)(4))))->to_expr()->equals(NEW(FunExpr)("x", NEW(AddExpr)(NEW(VarExpr)("x"), NEW(NumExpr)(4)))));
    
    /* add_to() */
    CHECK_THROWS_WITH((NEW(FunVal)("x", NEW(AddExpr)(NEW(VarExpr)("x"), NEW(NumExpr)(4))))->add_to(NEW(BoolVal)(true)), "Non-NumVal object detected. Cannot perform addition.");
    
    /* mult_by() */
    CHECK_THROWS_WITH((NEW(FunVal)("x", NEW(AddExpr)(NEW(VarExpr)("x"), NEW(NumExpr)(4))))->mult_by(NEW(BoolVal)(true)), "Non-NumVal object detected. Cannot perform multiplication.");
    /* is_true() */
    CHECK_THROWS_WITH((NEW(FunVal)("x", NEW(AddExpr)(NEW(VarExpr)("x"), NEW(NumExpr)(4))))->is_true(), "Non-BoolVal object detected.");
    
    /* call() */
    CHECK((NEW(FunVal)("x", NEW(AddExpr)(NEW(VarExpr)("x"), NEW(NumExpr)(4))))->call(NEW(NumVal)(4))->equals(NEW(NumVal)(8)));
    CHECK_THROWS_WITH((NEW(FunVal)("x", NEW(AddExpr)(NEW(VarExpr)("x"), NEW(NumExpr)(4))))->call(NEW(BoolVal)(true)), "Non-NumVal object detected. Cannot perform addition.");
    CHECK((NEW(FunVal)("x", NEW(AddExpr)(NEW(VarExpr)("x"), NEW(NumExpr)(4))))->call((NEW(FunVal)("x", NEW(AddExpr)(NEW(VarExpr)("x"), NEW(NumExpr)(4))))->call(NEW(NumVal)(3)))->equals(NEW(NumVal)(11)));
    
    CHECK((NEW(FunVal)("x", NEW(FunExpr)("y", NEW(AddExpr)(NEW(VarExpr)("x"), NEW(VarExpr)("y")))))->call(NEW(NumVal)(5))->equals((NEW(FunExpr)("y", NEW(AddExpr)(NEW(VarExpr)("x"), NEW(VarExpr)("y"))))->interp()));
}
