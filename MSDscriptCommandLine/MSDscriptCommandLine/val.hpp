//
//  val.hpp
//  MSDscriptCommandLine
//
//  Created by Michael Langston on 3/1/21.
//

#ifndef val_hpp
#define val_hpp

#include <stdio.h>

//Prototypes
class Expr;

class Val{
public:
    virtual bool equals(Val* v) = 0;
    virtual Expr* to_expr() = 0;
    virtual Val* add_to(Val* other_val) = 0;
    virtual Val* mult_by(Val* other_val) = 0;
    virtual bool is_true() = 0;
    
};






class NumVal : public Val {
public:
    int rep;
    
    /* Default Constructor */
    NumVal(int rep);
    
    /* Methods */
    bool equals(Val* v);
    Expr* to_expr();
    Val* add_to(Val* other_val);
    Val* mult_by(Val* other_val);
    bool is_true();
};





class BoolVal : public Val {
public:
    bool rep;
    
    /* Default Constructor */
    BoolVal(bool rep);
    
    /* Methods */
    bool equals(Val* v);
    Expr* to_expr();
    Val* add_to(Val* other_val);
    Val* mult_by(Val* other_val);
    bool is_true();
};

#endif /* val_hpp */
