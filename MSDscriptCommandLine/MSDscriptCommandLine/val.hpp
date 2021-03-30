//
//  val.hpp
//  MSDscriptCommandLine
//
//  Created by Michael Langston on 3/1/21.
//

#ifndef val_hpp
#define val_hpp

#include <stdio.h>
#include <string>
#include "pointermgmt.h"


//Prototypes
class Expr;
class Env;

CLASS(Val){
public:
    /* Override Methods */
    virtual bool equals(PTR(Val) v) = 0;
    virtual PTR(Val) add_to(PTR(Val) other_val) = 0;
    virtual PTR(Val) mult_by(PTR(Val) other_val) = 0;
    virtual bool is_true() = 0;
    virtual PTR(Val) call(PTR(Val) actual_arg) = 0;
    virtual std::string make_string() = 0;
    
    /* Destructor */
    virtual ~Val(){}
};

class NumVal : public Val {
public:
    /* Member Variables */
    int rep;
    
    /* Default Constructor */
    NumVal(int rep);
    
    /* Methods */
    bool equals(PTR(Val) v);
    PTR(Val) add_to(PTR(Val) other_val);
    PTR(Val) mult_by(PTR(Val) other_val);
    bool is_true();
    PTR(Val) call(PTR(Val) actual_arg);
    std::string make_string();
};

class BoolVal : public Val {
public:
    /* Member Variables */
    bool rep;
    
    /* Default Constructor */
    BoolVal(bool rep);
    
    /* Methods */
    bool equals(PTR(Val) v);
    PTR(Val) add_to(PTR(Val) other_val);
    PTR(Val) mult_by(PTR(Val) other_val);
    bool is_true();
    PTR(Val) call(PTR(Val) actual_arg);
    std::string make_string();
};

class FunVal : public Val {
public:
    /* Member Variables */
    std::string formal_arg;
    PTR(Expr) body;
    PTR(Env) env;
    
    /* Default Constructor */
    FunVal(std::string formal_arg, PTR(Expr) body, PTR(Env) env);
    
    /* Methods */
    bool equals(PTR(Val) v);
    PTR(Val) add_to(PTR(Val) other_val);
    PTR(Val) mult_by(PTR(Val) other_val);
    bool is_true();
    PTR(Val) call(PTR(Val) actual_arg);
    std::string make_string();
};

#endif /* val_hpp */
