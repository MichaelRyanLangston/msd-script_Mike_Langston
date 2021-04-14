//
//  continue.cpp
//  MSDscriptCommandLine
//
//  Created by Michael Langston on 4/11/21.
//

#include "continue.hpp"

/* Cont Implementation */
PTR(Cont) Cont::done = NEW(DoneCont)();

/* DoneCont Implementation */
void DoneCont::step_continue(){
    //Don't need to do anything here.
}

/* RightThenAddCont Implementation */

//Constructor
RightThenAddCont::RightThenAddCont(PTR(Expr) rhs, PTR(Env) env, PTR(Cont) rest){
    this->rhs = rhs;
    this->env = env;
    this->rest = rest;
}

//Methods
void RightThenAddCont::step_continue(){
    PTR(Val) lhs_val = Step::val;
    Step::mode = Step::interp_mode;
    Step::expr = rhs;
    Step::env = env;
    Step::cont = NEW(AddCont) (lhs_val, rest);
}

/* AddCont Implementation */

//Constructor
AddCont::AddCont(PTR(Val) lhs_val, PTR(Cont) rest){
    this->lhs_val = lhs_val;
    this->rest = rest;
}

//Methods
void AddCont::step_continue(){
    PTR(Val) rhs_val = Step::val;
    Step::mode = Step::continue_mode;
    Step::val = lhs_val->add_to(rhs_val);
    Step::cont = rest;
}

/* RightThenMultCont Implementation */

//Constructor
RightThenMultCont::RightThenMultCont(PTR(Expr) rhs, PTR(Env) env, PTR(Cont) rest){
    this->rhs = rhs;
    this->env = env;
    this->rest = rest;
}

//Methods
void RightThenMultCont::step_continue(){
    PTR(Val) lhs_val = Step::val;
    Step::mode = Step::interp_mode;
    Step::expr = rhs;
    Step::env = env;
    Step::cont = NEW(MultCont) (lhs_val, rest);
}

/* MultCont Implementation */

//Constructor
MultCont::MultCont(PTR(Val) lhs_val, PTR(Cont) rest){
    this->lhs_val = lhs_val;
    this->rest = rest;
}

//Methods
void MultCont::step_continue(){
    PTR(Val) rhs_val = Step::val;
    Step::mode = Step::continue_mode;
    Step::val = lhs_val->mult_by(rhs_val);
    Step::cont = rest;
}

/* RightThenEqCont Implementation */

//Constructor
RightThenEqCont::RightThenEqCont(PTR(Expr) rhs, PTR(Env) env, PTR(Cont) rest){
    this->rhs = rhs;
    this->env = env;
    this->rest = rest;
}

//Methods
void RightThenEqCont::step_continue(){
    PTR(Val) lhs_val = Step::val;
    Step::mode = Step::interp_mode;
    Step::expr = rhs;
    Step::env = env;
    Step::cont = NEW(EqCont) (lhs_val, rest);
}

/* EqCont Implementation */

//Constructor
EqCont::EqCont(PTR(Val) lhs_val, PTR(Cont) rest){
    this->lhs_val = lhs_val;
    this->rest = rest;
}

//Methods
void EqCont::step_continue(){
    PTR(Val) rhs_val = Step::val;
    Step::mode = Step::continue_mode;
    Step::val = NEW(BoolVal)(lhs_val->equals(rhs_val));
    Step::cont = rest;
}

/* IfBranchCont Implementation */

//Constructor
IfBranchCont:: IfBranchCont(PTR(Expr) then_part, PTR(Expr) else_part, PTR(Env) env, PTR(Cont) rest){
    this->then_part = then_part;
    this->else_part = else_part;
    this->env = env;
    this->rest = rest;
}

//Methods
void IfBranchCont::step_continue(){
    PTR(Val) test_val = Step::val;
    Step::mode = Step::interp_mode;
    if (test_val->is_true())
        Step::expr = then_part;
    else
        Step::expr = else_part;
    Step::env = env;
    Step::cont = rest;
}


/* LetBodyCont Implementation */

//Constructor
LetBodyCont::LetBodyCont(std::string var, PTR(Expr) body, PTR(Env) env, PTR(Cont) rest){
    this->var = var;
    this->body = body;
    this->env = env;
    this->rest = rest;
}

//Methods
void LetBodyCont::step_continue(){
    Step::mode = Step::interp_mode;
    Step::expr = body;
    Step::env = NEW(ExtendedEnv) (var, Step::val, env);
    Step::cont = rest;
}


/* ArgThenCallCont Implementation */

//Constructor
ArgThenCallCont::ArgThenCallCont(PTR(Expr) actual_arg, PTR(Env) env, PTR(Cont) rest){
    this->actual_arg = actual_arg;
    this->env = env;
    this->rest = rest;
}

//Methods
void ArgThenCallCont::step_continue(){
    Step::mode = Step::interp_mode;
    Step::expr = actual_arg;
    Step::env = env;
    Step::cont = NEW(CallCont)(Step::val, rest);
}

/* CallCount Implementation */

//Constructor
CallCont::CallCont(PTR(Val) to_be_called_val, PTR(Cont) rest){
    this->to_be_called_val = to_be_called_val;
    this->rest = rest;
}

//Methods
void CallCont::step_continue(){
    to_be_called_val->call_step(Step::val, rest);
}
