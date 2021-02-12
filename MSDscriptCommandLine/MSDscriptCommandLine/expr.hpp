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

//Enumeration for pretty print at with alias print_mode_t
//print_group_none => previous layer needs no parenthasis around it
//print_group_add => previous layer needed parenthasis
//print_group_add_or_mult => previous layer needed parenthasis round add or a mult
typedef enum {
    print_group_none,
    print_group_add,
    print_group_add_or_mult,
} print_mode_t;

//Expr acts as a Java Interface, so it is meant to be implemented by other classes or rather sub-classes since C++ doesn't have a notion of an interface.
class Expr {
public:
    // = 0 syntax means that each subclass must override it.
    virtual bool equals(Expr *e) = 0;
    virtual int interp() = 0;
    virtual bool has_variable() = 0;
    virtual Expr* subst(std::string s, Expr *e) = 0;
    virtual void print(std::ostream& out) = 0;
//    virtual void pretty_print_at(std::ostream& out, print_mode_t mode, int num) = 0;
    
    //Methods
    std::string to_string();
//    void pretty_print(std::ostream& out);
};


class Num : public Expr {
public:
    //Member Variables
    int val;
    
    //Default Constructor
    Num(int val);
    
    //Methods
    bool equals(Expr *e);
    int interp();
    bool has_variable();
    Expr* subst(std::string s, Expr *e);
    void print(std::ostream& out);
//    void pretty_print_at(std::ostream& out, print_mode_t mode, int num);
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
    int interp();
    bool has_variable();
    Expr* subst(std::string s, Expr *e);
    void print(std::ostream& out);
//    void pretty_print_at(std::ostream& out, print_mode_t mode, int num);
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
    int interp();
    bool has_variable();
    Expr* subst(std::string s, Expr *e);
    void print(std::ostream& out);
//    void pretty_print_at(std::ostream& out, print_mode_t mode, int num);
};

class Var : public Expr {
public:
    std::string var;
    
    //Default Constructor
    Var(std::string val);
    
    //Methods
    bool equals(Expr *e);
    int interp();
    bool has_variable();
    Expr* subst(std::string s, Expr *e);
    void print(std::ostream& out);
//    void pretty_print_at(std::ostream& out, print_mode_t mode, int num);
};

class _let : public Expr {
public:
    std::string lhs_name;
    Expr *rhs;
    Expr *body;
    
    //Default Constructor
    _let(std::string lhs_name, Expr *rhs,Expr *body);
    
    //Methods
    bool equals(Expr *e);
    int interp();
    bool has_variable();
    Expr* subst(std::string s, Expr *e);
    void print(std::ostream& out);
//    void pretty_print_at(std::ostream& out, print_mode_t mode, int num);
};

#endif /* expr_hpp */
