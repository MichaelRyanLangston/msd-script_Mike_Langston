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
#include "pointermgmt.h"

//Prototypes
CLASS(Val);

//Expr acts as a Java Interface, so it is meant to be implemented by other classes or rather sub-classes since C++ doesn't have a notion of an interface.
CLASS(Expr) {
public:
    /* Override Methods */
    // = 0 syntax means that each subclass must override it.
    virtual bool equals(PTR(Expr) e) = 0;
    virtual PTR(Val) interp() = 0;
    virtual bool has_variable() = 0;
    virtual PTR(Expr) subst(std::string s, PTR(Expr) e) = 0;
    virtual void print(std::ostream& out) = 0;
    
    
    //Methods
    std::string to_string();
    
    /*Depricated*/
    //    void pretty_print(std::ostream& out);
    //    virtual void pretty_print_at(std::ostream& out, print_mode_t mode, int num) = 0;
    /*Depricated*/
    ////Enumeration for pretty print at with alias print_mode_t
    ////print_group_none => previous layer needs no parenthasis around it
    ////print_group_add => previous layer needed parenthasis
    ////print_group_add_or_mult => previous layer needed parenthasis round add or a mult
    //typedef enum {
    //    print_group_none,
    //    print_group_add,
    //    print_group_add_or_mult,
    //} print_mode_t;
};

//Parsing Helper Functions
void consume_character(std::istream &in, int expected);
void skip_whitespace(std::istream &in);
void parse_keyword(std::istream &to_Parse, std::string expected_keyword);
PTR(Expr) parse_num(std::istream &to_Parse);
PTR(Expr) parse_var(std::istream &to_Parse);
PTR(Expr) parse_let(std::istream &to_Parse);
PTR(Expr) parse_if(std::istream &to_Parse);
PTR(Expr) parse_fun(std::istream &to_Parse);

//Parsing Functions
PTR(Expr) parse_expr(std::istream & to_Parse);
PTR(Expr) parse_comparg(std::istream & to_Parse);
PTR(Expr) parse_addend(std::istream &to_Parse);
PTR(Expr) parse_multicand(std::istream &to_Parse);
PTR(Expr) parse_inner(std::istream &to_Parse);







CLASS(NumExpr) : public Expr {
public:
    //Member Variables
    int rep;
    
    //Default Constructor
    NumExpr(int rep);
    
    //Methods
    bool equals(PTR(Expr)e);
    PTR(Val) interp();
    bool has_variable();
    PTR(Expr) subst(std::string s, PTR(Expr)e);
    void print(std::ostream& out);
};




CLASS(AddExpr) : public Expr {
public:
    //Member Variables
    PTR(Expr)lhs;
    PTR(Expr)rhs;
    
    //Default Constructor
    AddExpr(PTR(Expr)lhs, PTR(Expr)rhs);
    
    //Methods
    bool equals(PTR(Expr)e);
    PTR(Val) interp();
    bool has_variable();
    PTR(Expr) subst(std::string s, PTR(Expr)e);
    void print(std::ostream& out);
};




CLASS(MultExpr) : public Expr {
public:
    //Member Variables
    PTR(Expr) lhs;
    PTR(Expr) rhs;
    
    //Default Constructor
    MultExpr(PTR(Expr) lhs, PTR(Expr) rhs);
    
    //Methods
    bool equals(PTR(Expr)e);
    PTR(Val) interp();
    bool has_variable();
    PTR(Expr) subst(std::string s, PTR(Expr) e);
    void print(std::ostream& out);
};




CLASS(VarExpr) : public Expr {
public:
    std::string var;
    
    //Default Constructor
    VarExpr(std::string val);
    
    //Methods
    bool equals(PTR(Expr)e);
    PTR(Val) interp();
    bool has_variable();
    PTR(Expr) subst(std::string s, PTR(Expr)e);
    void print(std::ostream& out);
};




CLASS(LetExpr) : public Expr {
public:
    std::string lhs_name;
    PTR(Expr)rhs;
    PTR(Expr)body;
    
    //Default Constructor
    LetExpr(std::string lhs_name, PTR(Expr)rhs,PTR(Expr)body);
    
    //Methods
    bool equals(PTR(Expr)e);
    PTR(Val) interp();
    bool has_variable();
    PTR(Expr) subst(std::string s, PTR(Expr)e);
    void print(std::ostream& out);
};

CLASS(BoolExpr) : public Expr {
public:
    //Member Variables
    bool rep;
    
    //Default Constructor
    BoolExpr(bool rep);
    
    //Methods
    bool equals(PTR(Expr)e);
    PTR(Val) interp();
    bool has_variable();
    PTR(Expr) subst(std::string s, PTR(Expr)e);
    void print(std::ostream& out);
};

CLASS(EqExpr) : public Expr {
public:
    //Member Variables
    PTR(Expr) lhs;
    PTR(Expr) rhs;
    
    //Default Constructor
    EqExpr(PTR(Expr) lhs, PTR(Expr) rhs);
    
    //Methods
    bool equals(PTR(Expr)e);
    PTR(Val) interp();
    bool has_variable();
    PTR(Expr) subst(std::string s, PTR(Expr) e);
    void print(std::ostream& out);
};

CLASS(IfExpr) : public Expr {
public:
    //Member Variables
    PTR(Expr) comparison;
    PTR(Expr) if_true;
    PTR(Expr) if_false;
    
    //Default Constructor
    IfExpr(PTR(Expr) comparison, PTR(Expr) if_true, PTR(Expr) if_false);
    
    //Methods
    bool equals(PTR(Expr)e);
    PTR(Val) interp();
    bool has_variable();
    PTR(Expr) subst(std::string s, PTR(Expr)e);
    void print(std::ostream& out);
};

CLASS(FunExpr) : public Expr {
public:
    //Member Variables
    std::string formal_arg;
    PTR(Expr) body;
    
    //Default Constructor
    FunExpr(std::string formal_arg, PTR(Expr) body);
    
    //Methods
    bool equals(PTR(Expr) e);
    PTR(Val) interp();
    bool has_variable();
    PTR(Expr) subst(std::string s, PTR(Expr) e);
    void print(std::ostream& out);
};

CLASS(CallExpr) : public Expr {
public:
    //Member Variables
    PTR(Expr) to_be_called;
    PTR(Expr) actual_arg;
    
    //Default Constructor
    CallExpr(PTR(Expr) to_be_called, PTR(Expr) actual_arg);
    
    //Methods
    bool equals(PTR(Expr)e);
    PTR(Val) interp();
    bool has_variable();
    PTR(Expr) subst(std::string s, PTR(Expr)e);
    void print(std::ostream& out);
};

#endif /* expr_hpp */
