//
//  val.cpp
//  MSDscriptCommandLine
//
//  Created by Michael Langston on 3/1/21.
//

#include "val.hpp"
#include "expr.hpp"
#include "catch.h"

/* NumVal Implementation */

//Default Constructor
NumVal::NumVal(int rep){
    this->rep = rep;
}


//Methods
bool NumVal::equals(Val* v){
    NumVal* nv = dynamic_cast<NumVal*>(v);
    if (nv == NULL)
        return false;
    else
        return this->rep == nv->rep;
}

Expr* NumVal::to_expr(){
    return new NumExpr(this->rep);
}

Val* NumVal::add_to(Val* other_val){
    NumVal* other_num = dynamic_cast<NumVal*>(other_val);
    if(other_num == NULL)
        throw std::runtime_error("Non-NumVal object detected. Cannot perform addition.");
    return new NumVal(this->rep + other_num->rep);
}

Val* NumVal::mult_by(Val* other_val){
    NumVal* other_num = dynamic_cast<NumVal*>(other_val);
    if(other_num == NULL)
        throw std::runtime_error("Non-NumVal object detected. Cannot perform multiplication.");
    return new NumVal(this->rep * other_num->rep);
}

bool NumVal::is_true(){
    throw std::runtime_error("Non-BoolVal object detected.");
}

Val* NumVal::call(Val* actual_arg){
    throw std::runtime_error("Non-FunVal object detected.");
}

//Testing
TEST_CASE("NumVal Tests"){
    
    /* equals() */
    CHECK((new NumVal(0))->equals(new NumVal(0)));
    CHECK(!((new NumVal(1))->equals(new NumVal(2))));
    CHECK(!((new NumVal(2))->equals(new BoolVal(true))));
    
    /* to_expr() */
    CHECK((new NumVal(3))->to_expr()->equals(new NumExpr(3)));
    
    /* add_to() */
    CHECK((new NumVal(4))->add_to(new NumVal(1))->equals(new NumVal(5)));
    CHECK_THROWS_WITH((new NumVal(4))->add_to(new BoolVal(true)), "Non-NumVal object detected. Cannot perform addition.");
    
    /* mult_by() */
    CHECK((new NumVal(4))->mult_by(new NumVal(1))->equals(new NumVal(4)));
    CHECK_THROWS_WITH((new NumVal(4))->mult_by(new BoolVal(true)), "Non-NumVal object detected. Cannot perform multiplication.");
    
    /* is_true() */
    CHECK_THROWS_WITH((new NumVal(4))->is_true(), "Non-BoolVal object detected.");
    
    /* call() */
    CHECK_THROWS_WITH((new NumVal(4))->call(new NumVal(4)), "Non-FunVal object detected.");
}





/* BoolVal Implentation */
//Default Constructor
BoolVal::BoolVal(bool rep){
    this->rep = rep;
}

//Methods
bool BoolVal::equals(Val* v){
    BoolVal* bv = dynamic_cast<BoolVal*>(v);
    if (bv == NULL)
        return false;
    else
        return this->rep == bv->rep;
}

Expr* BoolVal::to_expr(){
    return new BoolExpr(this->rep);
}

Val* BoolVal::add_to(Val* other_val){
    throw std::runtime_error("Non-NumVal object detected. Cannot perform addition.");
}

Val* BoolVal::mult_by(Val* other_val){
    throw std::runtime_error("Non-NumVal object detected. Cannot perform multiplication.");
}

bool BoolVal::is_true(){
    return this->rep;
}

Val* BoolVal::call(Val* actual_arg){
    throw std::runtime_error("Non-FunVal object detected.");
}

TEST_CASE("BoolVal Tests"){
    /* equals() */
    CHECK((new BoolVal(false))->equals(new BoolVal(false)));
    CHECK(!((new BoolVal(true))->equals(new BoolVal(false))));
    CHECK(!((new BoolVal(true))->equals(new NumVal(0))));
    
    /* to_expr()*/
    CHECK((new BoolVal(true))->to_expr()->equals(new BoolExpr(true)));
    CHECK((new BoolVal(false))->to_expr()->equals(new BoolExpr(false)));
    
    /* add_to() */
    CHECK_THROWS_WITH((new BoolVal(true))->add_to(new NumVal(false)), "Non-NumVal object detected. Cannot perform addition.");
    
    /* mult_by() */
    CHECK_THROWS_WITH((new BoolVal(true))->mult_by(new NumVal(false)),"Non-NumVal object detected. Cannot perform multiplication.");
    
    /* is_true() */
    CHECK((new BoolVal(true))->is_true());
    CHECK(!((new BoolVal(false))->is_true()));
    
    /* call() */
    CHECK_THROWS_WITH((new BoolVal(true))->call(new BoolVal(true)), "Non-FunVal object detected.");
}





/* FunVal Implementations */
//Default Constructor
FunVal::FunVal(std::string formal_arg, Expr* body){
    this->formal_arg = formal_arg;
    this->body = body;
}

//Methods
bool FunVal::equals(Val* v){
    FunVal* fv = dynamic_cast<FunVal*>(v);
    if (fv == NULL)
        return false;
    else
        return this->formal_arg == fv->formal_arg && this->body->equals(fv->body);
}

Expr* FunVal::to_expr(){
    return new FunExpr(this->formal_arg, this->body);
}

Val* FunVal::add_to(Val* other_val){
    throw std::runtime_error("Non-NumVal object detected. Cannot perform addition.");
}

Val* FunVal::mult_by(Val* other_val){
    throw std::runtime_error("Non-NumVal object detected. Cannot perform multiplication.");
}

bool FunVal::is_true(){
    throw std::runtime_error("Non-BoolVal object detected.");
}

Val* FunVal::call(Val* actual_arg){
    return this->body->subst(formal_arg, actual_arg->to_expr())->interp();
}

TEST_CASE("FunVal Tests"){
    /* equals() */
    
    /* to_expr()*/
    
    /* add_to() */
    
    /* mult_by() */
    
    /* is_true() */
    
    /* call() */
    
}
