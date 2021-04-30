//
//  step.hpp
//  MSDscriptCommandLine
//
//  Created by Michael Langston on 4/11/21.
//

/* Step is used to avoid stack overflow that could happen by to many recursive calls in the general interp mode. This is the recommend mode for recrsive programs */
#ifndef step_hpp
#define step_hpp

#include <stdio.h>
#include "pointermgmt.h"
#include "parse.hpp"
#include "expr.hpp"
#include "val.hpp"
#include "env.hpp"
#include "continue.hpp"


//Prototype
class Cont;
class Expr;
class Step;
class Val;


CLASS(Step){
public:
    typedef enum{
        interp_mode,
        continue_mode
    } mode_t;
    
    /* Global Variables */
    static mode_t mode;
    static PTR(Expr) expr;
    static PTR(Env) env;
    static PTR(Val) val;
    static PTR(Cont) cont;
    
    /* Methods */
    static PTR(Val) interp_by_steps(PTR(Expr) e);
    
    /* Destructor */
    virtual ~Step(){}
};



#endif /* step_hpp */
