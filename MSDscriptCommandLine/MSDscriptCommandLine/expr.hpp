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
//three kinds of expressions: number, addition of two expressions, multiplication of two epressions.

//Expr acts as a Java Interface, so it is meant to be implemented by other classes or rather sub-classes since C++ doesn't have a notion of an interface.
class Expr {
public:
    // = 0 syntax means that each subclass must override it.
    virtual bool equals(Expr *e) = 0;
};

class Num : public Expr {
public:
    //Member Variables
    int val;
    
    //Default Constructor
    Num(int val);
    
    //Methods
    bool equals(Expr *e);
};

class Add : public Expr {
public:
    //Member Variables
    Expr *lhs;
    Expr *rhs;
    
    //Default Constructor
    Add(Expr *lhs, Expr *rhs);
    
    //Methods
    bool equals(Expr *e);
};

class Mult : public Expr {
public:
    //Member Variables
    Expr *lhs;
    Expr *rhs;
    
    //Default Constructor
    Mult(Expr *lhs, Expr *rhs);
    
    //Methods
    bool equals(Expr *e);
};

class Var : public Expr {
public:
    std::string val;
    
    //Default Constructor
    Var(std::string val);
    
    //Methods
    bool equals(Expr *e);
};
#endif /* expr_hpp */
