//
//  continue.hpp
//  MSDscriptCommandLine
//
//  Created by Michael Langston on 4/11/21.
//

/* The continue class is used in step mode to move between steps. When contiue is called the program executes the continuation and then will move to the next step in the progarm. This aids in avoiding storage on the call stack. */

#ifndef continue_hpp
#define continue_hpp

#include <stdio.h>
#include "pointermgmt.h"
#include "expr.hpp"
#include "val.hpp"
#include "env.hpp"
#include "step.hpp"

//Prototype
class Expr;
class Val;
class Env;
class Step;

CLASS(Cont){
public:
    /* Methods */
    virtual void step_continue() = 0;
    
    /* Global Variables */
    static PTR(Cont) done;
    
    /* Destructor */
    virtual ~Cont(){}
};

class DoneCont: public Cont{
public:
    /* Member Variables */
    //NA
    
    /* Default Constructor */
    //NA
    
    /* Methods */
    void step_continue();
};


class RightThenAddCont : public Cont{
public:
    /* Member Variables */
    PTR(Expr) rhs;
    PTR(Env) env;
    PTR(Cont) rest;
    
    
    /* Constructor */
    RightThenAddCont(PTR(Expr) rhs, PTR(Env), PTR(Cont) rest);
    
    /* Methods */
    void step_continue();
};

class AddCont : public Cont{
public:
    /* Member Variables */
    PTR(Val) lhs_val;
    PTR(Cont) rest;
    
    /* Constructor*/
    AddCont(PTR(Val) lhs_val, PTR(Cont) rest);
    
    /* Methods */
    void step_continue();
};

class RightThenMultCont : public Cont{
public:
    /* Member Variables */
    PTR(Expr) rhs;
    PTR(Env) env;
    PTR(Cont) rest;
    
    
    /* Constructor */
    RightThenMultCont(PTR(Expr) rhs, PTR(Env), PTR(Cont) rest);
    
    /* Methods */
    void step_continue();
};

class MultCont : public Cont{
public:
    /* Member Variables */
    PTR(Val) lhs_val;
    PTR(Cont) rest;
    
    /* Constructor*/
    MultCont(PTR(Val) lhs_val, PTR(Cont) rest);
    
    /* Methods */
    void step_continue();
};

class RightThenEqCont : public Cont{
public:
    /* Member Variables */
    PTR(Expr) rhs;
    PTR(Env) env;
    PTR(Cont) rest;
    
    
    /* Constructor */
    RightThenEqCont(PTR(Expr) rhs, PTR(Env), PTR(Cont) rest);
    
    /* Methods */
    void step_continue();
};

class EqCont : public Cont{
public:
    /* Member Variables */
    PTR(Val) lhs_val;
    PTR(Cont) rest;
    
    /* Constructor*/
    EqCont(PTR(Val) lhs_val, PTR(Cont) rest);
    
    /* Methods */
    void step_continue();
};

class IfBranchCont : public Cont{
public:
    /* Member Variables */
    PTR(Expr) then_part;
    PTR(Expr) else_part;
    PTR(Env) env;
    PTR(Cont) rest;
    
    /* Constructor*/
    IfBranchCont(PTR(Expr) then_part, PTR(Expr) else_part, PTR(Env) env, PTR(Cont) rest);
    
    /* Methods */
    void step_continue();
};

class LetBodyCont : public Cont{
public:
    /* Member Variables */
    std::string var;
    PTR(Expr) body;
    PTR(Env) env;
    PTR(Cont) rest;
    
    /* Constructor */
    LetBodyCont(std::string var, PTR(Expr) body, PTR(Env) env, PTR(Cont) rest);
    
    /* Methods */
    void step_continue();
};

class ArgThenCallCont : public Cont{
public:
    /* Member Variables */
    PTR(Expr) actual_arg;
    PTR(Env) env;
    PTR(Cont) rest;
    
    /* Constructor */
    ArgThenCallCont(PTR(Expr) actual_arg, PTR(Env) env, PTR(Cont) rest);
    
    /* Methods */
    void step_continue();
};

class CallCont : public Cont{
public:
    /* Member Variables */
    PTR(Val) to_be_called_val;
    PTR(Cont) rest;
    
    /* Constructor */
    CallCont(PTR(Val) to_be_called_val, PTR(Cont) rest);
    
    /* Methods */
    void step_continue();
};

#endif /* continue_hpp */
