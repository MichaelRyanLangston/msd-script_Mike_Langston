//
//  env.cpp
//  MSDscriptCommandLine
//
//  Created by Michael Langston on 3/29/21.
//

#include "env.hpp"


/* Env */
PTR(Env) Env::empty = NEW(EmptyEnv)();


/* EmptyEnv */
//Methods
PTR(Val) EmptyEnv::lookup(std::string find_name){
    throw std::runtime_error("free variable detected: " + find_name);
}


/* ExtendedEnv */
//Default Constructor
ExtendedEnv::ExtendedEnv(std::string name, PTR(Val) val, PTR(Env) rest){
    this->name = name;
    this->val = val;
    this->rest = rest;
}

//Methods
PTR(Val) ExtendedEnv::lookup(std::string find_name){
    if (find_name == name)
        return val;
    else
        return rest->lookup(find_name);
}
