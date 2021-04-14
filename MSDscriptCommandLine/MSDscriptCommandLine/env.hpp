//
//  env.hpp
//  MSDscriptCommandLine
//
//  Created by Michael Langston on 3/29/21.
//

#ifndef env_hpp
#define env_hpp

#include <stdio.h>
#include <string>
#include <sstream>
#include "pointermgmt.h"


//Prototypes
class Expr;
class Val;
class Step;
class Cont;

CLASS(Env){
public:
    /* Override Methods */
    virtual PTR(Val) lookup(std::string find_name) = 0;
    
    /* Global Variables */
    static PTR(Env) empty;
    
    /* Destructor */
    virtual ~Env(){}
};

class EmptyEnv : public Env {
public:
    /* Member Variables */
    //NA
    
    /* Default Constructor */
    //NA
    
    /* Methods */
    PTR(Val) lookup(std::string find_name);
    
};

class ExtendedEnv : public Env {
public:
    /* Member Variables */
    std::string name;
    PTR(Val) val;
    PTR(Env) rest;
    
    /* Default Constructor */
    ExtendedEnv(std::string name, PTR(Val) val, PTR(Env) rest);
    
    /* Methods */
    PTR(Val) lookup(std::string find_name);
};

#endif /* env_hpp */
