//
//  val.cpp
//  MSDscriptCommandLine
//
//  Created by Michael Langston on 3/1/21.
//

#include "val.hpp"


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

PTR(Val) NumVal::add_to(PTR(Val) other_val){
    PTR(NumVal) other_num = CAST(NumVal)(other_val);
    if(other_num == NULL)
        throw std::runtime_error("Non-NumVal object detected. Cannot perform addition.");
    //Below Checks to see if overflow is occuring...
    if (this->rep >= 0 && other_num->rep >= 0){
        if (this->rep + other_num->rep < 0){
            throw std::runtime_error("This number cannot be represented by the backing type 'int' becasue it causes memory overflow...");
        }
    }
    else if (this->rep < 0 && other_num->rep < 0){
        if (this->rep + other_num->rep >= 0){
            throw std::runtime_error("This number cannot be represented by the backing type 'int' becasue it causes memory overflow...");
        }
    }
    return NEW(NumVal)(this->rep + other_num->rep);
}

PTR(Val) NumVal::mult_by(PTR(Val) other_val){
    PTR(NumVal) other_num = CAST(NumVal)(other_val);
    if(other_num == NULL)
        throw std::runtime_error("Non-NumVal object detected. Cannot perform multiplication.");
    
    /* The __builtin_mul_overflow() takes the first two areguments and casts them to infinate precision signed type, mutiplies them, stores it in the pointer and identifies if result is equal to the casted value
     Source:https://gcc.gnu.org/onlinedocs/gcc/Integer-Overflow-Builtins.html
     Source:https://stackoverflow.com/questions/38625857/using-the-builtin-function-builtin-add-overflow-p-in-gcc
     */
//    int* y = 0;//This is a type checker, so the initial value doesn't matter.
//    if (__builtin_mul_overflow(this->rep,other_num->rep,y))
//        throw std::runtime_error("This number cannot be represented by the backing type 'int' becasue it causes memory overflow...");
    return NEW(NumVal)(this->rep * other_num->rep);
}

bool NumVal::is_true(){
    throw std::runtime_error("NumVal object detected. Cannot check for _true condition.");
}

PTR(Val) NumVal::call(PTR(Val) actual_arg){
    throw std::runtime_error("NumVal object detected. Cannot perform a call.");
}

void NumVal::call_step(PTR(Val) actual_arg_val, PTR(Cont) rest){
    throw std::runtime_error("NumVal object detected. Cannot perform a call.");
}

std::string NumVal::make_string(){
    return std::to_string(rep);
}

//Testing
TEST_CASE("NumVal Tests"){
    
    /* equals() */
    CHECK((NEW(NumVal)(0))->equals(NEW(NumVal)(0)));
    CHECK(!((NEW(NumVal)(1))->equals(NEW(NumVal)(2))));
    CHECK(!((NEW(NumVal)(2))->equals(NEW(BoolVal)(true))));
    
    /* add_to() */
    CHECK((NEW(NumVal)(4))->add_to(NEW(NumVal)(1))->equals(NEW(NumVal)(5)));
    CHECK_THROWS_WITH((NEW(NumVal)(4))->add_to(NEW(BoolVal)(true)), "Non-NumVal object detected. Cannot perform addition.");
    
    /* mult_by() */
    CHECK((NEW(NumVal)(4))->mult_by(NEW(NumVal)(1))->equals(NEW(NumVal)(4)));
    CHECK_THROWS_WITH((NEW(NumVal)(4))->mult_by(NEW(BoolVal)(true)), "Non-NumVal object detected. Cannot perform multiplication.");
    
    /* is_true() */
    CHECK_THROWS_WITH((NEW(NumVal)(4))->is_true(), "NumVal object detected. Cannot check for _true condition.");
    
    /* call() */
    CHECK_THROWS_WITH((NEW(NumVal)(4))->call(NEW(NumVal)(4)), "NumVal object detected. Cannot perform a call.");
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

PTR(Val) BoolVal::add_to(PTR(Val) other_val){
    throw std::runtime_error("BoolVal object detected. Cannot perform addition.");
}

PTR(Val) BoolVal::mult_by(PTR(Val) other_val){
    throw std::runtime_error("BoolVal object detected. Cannot perform multiplication.");
}

bool BoolVal::is_true(){
    return this->rep;
}

PTR(Val) BoolVal::call(PTR(Val) actual_arg){
    throw std::runtime_error("BoolVal object detected. Cannot perform a call.");
}

void BoolVal::call_step(PTR(Val) actual_arg_val, PTR(Cont) rest){
    throw std::runtime_error("BoolVal object detected. Cannot perform a call.");
}

std::string BoolVal::make_string(){
    if(rep)
        return "_true";
    return "_false";
}

TEST_CASE("BoolVal Tests"){
    /* equals() */
    CHECK((NEW(BoolVal)(false))->equals(NEW(BoolVal)(false)));
    CHECK(!((NEW(BoolVal)(true))->equals(NEW(BoolVal)(false))));
    CHECK(!((NEW(BoolVal)(true))->equals(NEW(NumVal)(0))));
    
    /* add_to() */
    CHECK_THROWS_WITH((NEW(BoolVal)(true))->add_to(NEW(NumVal)(false)), "BoolVal object detected. Cannot perform addition.");
    
    /* mult_by() */
    CHECK_THROWS_WITH((NEW(BoolVal)(true))->mult_by(NEW(NumVal)(false)),"BoolVal object detected. Cannot perform multiplication.");
    
    /* is_true() */
    CHECK((NEW(BoolVal)(true))->is_true());
    CHECK(!((NEW(BoolVal)(false))->is_true()));
    
    /* call() */
    CHECK_THROWS_WITH((NEW(BoolVal)(true))->call(NEW(BoolVal)(true)), "BoolVal object detected. Cannot perform a call.");
}





/* FunVal Implementations */
//Default Constructor
FunVal::FunVal(std::string formal_arg, PTR(Expr) body, PTR(Env) env){
    this->formal_arg = formal_arg;
    this->body = body;
    this->env = env;
}

//Methods
bool FunVal::equals(PTR(Val) v){
    PTR(FunVal) fv = CAST(FunVal)(v);
    if (fv == NULL)
        return false;
    else
        return this->formal_arg == fv->formal_arg && this->body->equals(fv->body);
}

PTR(Val) FunVal::add_to(PTR(Val) other_val){
    throw std::runtime_error("FunVal object detected. Cannot perform addition.");
}

PTR(Val) FunVal::mult_by(PTR(Val) other_val){
    throw std::runtime_error("FunVal object detected. Cannot perform multiplication.");
}

bool FunVal::is_true(){
    throw std::runtime_error("FunVal object detected. Cannot check for _true condition.");
}

PTR(Val) FunVal::call(PTR(Val) actual_arg){
    return body->interp(NEW(ExtendedEnv)(formal_arg, actual_arg, env));
}

void FunVal::call_step(PTR(Val) actual_arg_val, PTR(Cont) rest){
    Step::mode = Step::interp_mode;
    Step::expr = body;
    Step::env = NEW(ExtendedEnv) (formal_arg, actual_arg_val, env);
    Step::cont = rest;
}

std::string FunVal::make_string(){
    return "[function]";
}

TEST_CASE("FunVal Tests"){
    /* equals() */
    CHECK((NEW(FunVal)("x", NEW(AddExpr)(NEW(VarExpr)("x"), NEW(NumExpr)(4)), EmptyEnv::empty))->equals(NEW(FunVal)("x", NEW(AddExpr)(NEW(VarExpr)("x"), NEW(NumExpr)(4)), EmptyEnv::empty)));
    CHECK(!((NEW(FunVal)("x", NEW(AddExpr)(NEW(VarExpr)("x"), NEW(NumExpr)(4)), EmptyEnv::empty))->equals(NEW(FunVal)("y", NEW(AddExpr)(NEW(VarExpr)("x"), NEW(NumExpr)(4)), EmptyEnv::empty))));
    CHECK(!((NEW(FunVal)("x", NEW(AddExpr)(NEW(VarExpr)("x"), NEW(NumExpr)(4)), EmptyEnv::empty))->equals(NEW(FunVal)("x", NEW(MultExpr)(NEW(VarExpr)("x"), NEW(NumExpr)(4)), EmptyEnv::empty))));
    CHECK(!((NEW(FunVal)("x", NEW(AddExpr)(NEW(VarExpr)("x"), NEW(NumExpr)(4)), EmptyEnv::empty))->equals(NEW(BoolVal)(true))));
    
    /* add_to() */
    CHECK_THROWS_WITH((NEW(FunVal)("x", NEW(AddExpr)(NEW(VarExpr)("x"), NEW(NumExpr)(4)), EmptyEnv::empty))->add_to(NEW(BoolVal)(true)), "FunVal object detected. Cannot perform addition.");
    
    /* mult_by() */
    CHECK_THROWS_WITH((NEW(FunVal)("x", NEW(AddExpr)(NEW(VarExpr)("x"), NEW(NumExpr)(4)), EmptyEnv::empty))->mult_by(NEW(BoolVal)(true)), "FunVal object detected. Cannot perform multiplication.");
    /* is_true() */
    CHECK_THROWS_WITH((NEW(FunVal)("x", NEW(AddExpr)(NEW(VarExpr)("x"), NEW(NumExpr)(4)), EmptyEnv::empty))->is_true(), "FunVal object detected. Cannot check for _true condition.");
    
    /* call() */
    CHECK((NEW(FunVal)("x", NEW(AddExpr)(NEW(VarExpr)("x"), NEW(NumExpr)(4)), EmptyEnv::empty))->call(NEW(NumVal)(4))->equals(NEW(NumVal)(8)));
    CHECK_THROWS_WITH((NEW(FunVal)("x", NEW(AddExpr)(NEW(VarExpr)("x"), NEW(NumExpr)(4)), EmptyEnv::empty))->call(NEW(BoolVal)(true)), "BoolVal object detected. Cannot perform addition.");
    CHECK((NEW(FunVal)("x", NEW(AddExpr)(NEW(VarExpr)("x"), NEW(NumExpr)(4)), EmptyEnv::empty))->call((NEW(FunVal)("x", NEW(AddExpr)(NEW(VarExpr)("x"), NEW(NumExpr)(4)), EmptyEnv::empty))->call(NEW(NumVal)(3)))->equals(NEW(NumVal)(11)));
}
