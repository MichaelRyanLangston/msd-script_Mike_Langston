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


//Prototypes
class Val;


//Expr acts as a Java Interface, so it is meant to be implemented by other classes or rather sub-classes since C++ doesn't have a notion of an interface.
class Expr {
public:
    /* Override Methods */
    // = 0 syntax means that each subclass must override it.
    virtual bool equals(Expr *e) = 0;
    virtual Val* interp() = 0;
    virtual bool has_variable() = 0;
    virtual Expr* subst(std::string s, Expr *e) = 0;
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
    //} print*_mode_t;
};

//Parsing Helper Functions
void consume_character(std::istream &in, int expected);
void skip_whitespace(std::istream &in);
void parse_keyword(std::istream &to_Parse, std::string expected_keyword);
Expr* parse_num(std::istream &to_Parse);
Expr* parse_var(std::istream &to_Parse);
Expr* parse_let(std::istream &to_Parse);

//Parsing Functions
Expr* parse_multicand(std::istream &to_Parse);
Expr* parse_addend(std::istream &to_Parse);
Expr* parse_expr(std::istream & to_Parse);

class NumExpr : public Expr {
public:
    //Member Variables
    int rep;
    
    //Default Constructor
    NumExpr(int rep);
    
    //Methods
    bool equals(Expr *e);
    Val* interp();
    bool has_variable();
    Expr* subst(std::string s, Expr *e);
    void print(std::ostream& out);
};

class AddExpr : public Expr {
public:
    //Member Variables
    Expr *lhs;
    Expr *rhs;
    
    //Default Constructor
    AddExpr(Expr *lhs, Expr *rhs);
    
    //Methods
    bool equals(Expr *e);
    Val* interp();
    bool has_variable();
    Expr* subst(std::string s, Expr *e);
    void print(std::ostream& out);
};

class MultExpr : public Expr {
public:
    //Member Variables
    Expr *lhs;
    Expr *rhs;
    
    //Default Constructor
    MultExpr(Expr *lhs, Expr *rhs);
    
    //Methods
    bool equals(Expr *e);
    Val* interp();
    bool has_variable();
    Expr* subst(std::string s, Expr *e);
    void print(std::ostream& out);
};

class VarExpr : public Expr {
public:
    std::string var;
    
    //Default Constructor
    VarExpr(std::string val);
    
    //Methods
    bool equals(Expr *e);
    Val* interp();
    bool has_variable();
    Expr* subst(std::string s, Expr *e);
    void print(std::ostream& out);
};

class LetExpr : public Expr {
public:
    std::string lhs_name;
    Expr *rhs;
    Expr *body;
    
    //Default Constructor
    LetExpr(std::string lhs_name, Expr *rhs,Expr *body);
    
    //Methods
    bool equals(Expr *e);
    Val* interp();
    bool has_variable();
    Expr* subst(std::string s, Expr *e);
    void print(std::ostream& out);
};

#endif /* expr_hpp */
