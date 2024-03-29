//
//  expr.hpp
//  MSDscriptCommandLine
//
//  Created by Michael Langston on 1/24/21.
//

#ifndef expr_hpp
#define expr_hpp

#include <stdio.h>
#include <string>
#include <iostream>
#include <stdexcept>
#include <sstream>
#include <iostream>
#include "pointermgmt.h"
#include "val.hpp"
#include "env.hpp"
#include "step.hpp"
#include "continue.hpp"



//Prototypes
class Val;
class Env;
class Step;
class Cont;


//Expr acts as a Java Interface, so it is meant to be implemented by other classes or rather sub-classes since C++ doesn't have a notion of an interface.
CLASS(Expr) {
public:
    /* Override Methods:
            = 0 syntax means that each subclass must override it.
     */
    
    //evaluates if two expressions are equal
    virtual bool equals(PTR(Expr) e) = 0;
    
    //Determins how an expression should be interpeted in --interp mode
    virtual PTR(Val) interp(PTR(Env) env) = 0;
    
    //Determins how an expression should be interpeted in --step mode
    virtual void step_interp() = 0;
    
    //Determins how the specified expression is printed out to the console
    virtual void print(std::ostream& out) = 0;
    
    
    /* Class Methods */
    
    //Translates an expression to string
    std::string to_string();
    
    /* Destructor */
    virtual ~Expr(){}
    
};

class NumExpr : public Expr {
public:
    //Member Variables
    int rep;
    
    //Default Constructor
    NumExpr(int rep);
    
    //Methods
    bool equals(PTR(Expr)e);
    PTR(Val) interp(PTR(Env) env);
    void step_interp();
    void print(std::ostream& out);
};

class AddExpr : public Expr {
public:
    //Member Variables
    PTR(Expr)lhs;
    PTR(Expr)rhs;
    
    //Default Constructor
    AddExpr(PTR(Expr)lhs, PTR(Expr)rhs);
    
    //Methods
    bool equals(PTR(Expr)e);
    PTR(Val) interp(PTR(Env) env);
    void step_interp();
    void print(std::ostream& out);
};

class MultExpr : public Expr {
public:
    //Member Variables
    PTR(Expr) lhs;
    PTR(Expr) rhs;
    
    //Default Constructor
    MultExpr(PTR(Expr) lhs, PTR(Expr) rhs);
    
    //Methods
    bool equals(PTR(Expr)e);
    PTR(Val) interp(PTR(Env) env);
    void step_interp();
    void print(std::ostream& out);
};

class VarExpr : public Expr {
public:
    std::string var;
    
    //Default Constructor
    VarExpr(std::string val);
    
    //Methods
    bool equals(PTR(Expr)e);
    PTR(Val) interp(PTR(Env) env);
    void step_interp();
    void print(std::ostream& out);
};

class LetExpr : public Expr {
public:
    std::string lhs_name;
    PTR(Expr)rhs;
    PTR(Expr)body;
    
    //Default Constructor
    LetExpr(std::string lhs_name, PTR(Expr)rhs,PTR(Expr)body);
    
    //Methods
    bool equals(PTR(Expr)e);
    PTR(Val) interp(PTR(Env) env);
    void step_interp();
    void print(std::ostream& out);
};

class BoolExpr : public Expr {
public:
    //Member Variables
    bool rep;
    
    //Default Constructor
    BoolExpr(bool rep);
    
    //Methods
    bool equals(PTR(Expr)e);
    PTR(Val) interp(PTR(Env) env);
    void step_interp();
    void print(std::ostream& out);
};

class EqExpr : public Expr {
public:
    //Member Variables
    PTR(Expr) lhs;
    PTR(Expr) rhs;
    
    //Default Constructor
    EqExpr(PTR(Expr) lhs, PTR(Expr) rhs);
    
    //Methods
    bool equals(PTR(Expr)e);
    PTR(Val) interp(PTR(Env) env);
    void step_interp();
    void print(std::ostream& out);
};

class IfExpr : public Expr {
public:
    //Member Variables
    PTR(Expr) comparison;
    PTR(Expr) if_true;
    PTR(Expr) if_false;
    
    //Default Constructor
    IfExpr(PTR(Expr) comparison, PTR(Expr) if_true, PTR(Expr) if_false);
    
    //Methods
    bool equals(PTR(Expr)e);
    PTR(Val) interp(PTR(Env) env);
    void step_interp();
    void print(std::ostream& out);
};

class FunExpr : public Expr {
public:
    //Member Variables
    std::string formal_arg;
    PTR(Expr) body;
    
    //Default Constructor
    FunExpr(std::string formal_arg, PTR(Expr) body);
    
    //Methods
    bool equals(PTR(Expr) e);
    PTR(Val) interp(PTR(Env) env);
    void step_interp();
    void print(std::ostream& out);
};

class CallExpr : public Expr {
public:
    //Member Variables
    PTR(Expr) to_be_called;
    PTR(Expr) actual_arg;
    
    //Default Constructor
    CallExpr(PTR(Expr) to_be_called, PTR(Expr) actual_arg);
    
    //Methods
    bool equals(PTR(Expr)e);
    PTR(Val) interp(PTR(Env) env);
    void step_interp();
    void print(std::ostream& out);
};

#endif /* expr_hpp */
