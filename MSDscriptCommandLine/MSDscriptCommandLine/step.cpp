//
//  step.cpp
//  MSDscriptCommandLine
//
//  Created by Michael Langston on 4/11/21.
//

#include "step.hpp"

/* Step Implementation */

//Globabl Variables
Step::mode_t Step::mode;
PTR(Expr) Step::expr;
PTR(Env) Step::env;
PTR(Val) Step::val;
PTR(Cont) Step::cont;

//Methods
PTR(Val) Step::interp_by_steps(PTR(Expr) e){
    Step::mode = Step::interp_mode;
    Step::expr = e;
    Step::env = Env::empty;
    Step::val = nullptr;
    Step::cont = Cont::done;
    
    while (true) {
        if (Step::mode == Step::interp_mode)
            Step::expr->step_interp();
        else{
            if (Step::cont == Cont::done) {
                return Step::val;
            }
            else
                Step::cont->step_continue();
        }
    }
}


TEST_CASE("Interp_by_steps Testing"){
    //NumExper Step Testing
    std::stringstream testNum("1");
    CHECK(Step::interp_by_steps(parse_expr(testNum))->equals(NEW(NumVal)(1)));
    
    //BoolExper Step Testing
    std::stringstream testBoolTrue("_true");
    CHECK(Step::interp_by_steps(parse_expr(testBoolTrue))->equals(NEW(BoolVal)(true)));
    
    std::stringstream testBoolFalse("_false");
    CHECK(Step::interp_by_steps(parse_expr(testBoolFalse))->equals(NEW(BoolVal)(false)));
    
    //FunExper / LetExper / CallExper Step Testing
    std::stringstream testFun("_let f = _fun (x) x + 1 _in f(5)");
    CHECK(Step::interp_by_steps(parse_expr(testFun))->equals(NEW(NumVal)(6)));
    
    //IfExper Testing
    std::stringstream testIfTrue("_if 1 == 1 _then 4 _else 5");
    CHECK(Step::interp_by_steps(parse_expr(testIfTrue))->equals(NEW(NumVal)(4)));
    
    std::stringstream testIfFalse("_if 1 == 2 _then 4 _else 5");
    CHECK(Step::interp_by_steps(parse_expr(testIfFalse))->equals(NEW(NumVal)(5)));
    
    //AddExper StepTesting
    std::stringstream test1("1 + 2");
    CHECK(Step::interp_by_steps(parse_expr(test1))->equals(NEW(NumVal)(3)));
    
    //MultExper Step Testing
    std::stringstream test2("1 * 2");
    CHECK(Step::interp_by_steps(parse_expr(test2))->equals(NEW(NumVal)(2)));
    
    //EqExper Step Testing
    std::stringstream testEqFalse("1 == 2");
    CHECK(Step::interp_by_steps(parse_expr(testEqFalse))->equals(NEW(BoolVal)(false)));
    
    std::stringstream testEqTrue("1 == 1");
    CHECK(Step::interp_by_steps(parse_expr(testEqTrue))->equals(NEW(BoolVal)(true)));
    
    //Overall Step Testing
    std::stringstream testFactorial("_let factrl = _fun (factrl) _fun (x) _if x == 1 _then 1 _else x * factrl(factrl)(x + -1) _in  factrl(factrl)(10)");
    CHECK(Step::interp_by_steps(parse_expr(testFactorial))->equals(NEW(NumVal)(3628800)));
    
}
