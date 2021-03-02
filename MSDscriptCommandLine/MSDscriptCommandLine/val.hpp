//
//  val.hpp
//  MSDscriptCommandLine
//
//  Created by Michael Langston on 2/25/21.
//

#ifndef val_hpp
#define val_hpp

#include <stdio.h>

//Prototype
class Expr;


/*Abstract Class*/
class Val{
public:
    /*Method*/
    virtual Expr* to_expr() = 0;
    
};

class NumVal : public Val {
public:
    int number;
    
    /*Constructor*/
    NumVal(int value);
    
    /*Method*/
    Expr* to_expr();
};

#endif /* val_hpp */
