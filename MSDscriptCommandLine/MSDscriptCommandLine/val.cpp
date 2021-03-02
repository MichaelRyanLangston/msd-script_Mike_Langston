//
//  val.cpp
//  MSDscriptCommandLine
//
//  Created by Michael Langston on 2/25/21.
//

#include "val.hpp"
#include "expr.hpp"

/*Consturctor*/
NumVal::NumVal(int number){
    this->number = number;
}

/*Methods*/
Expr* NumVal::to_expr() {
    return new NumExpr(number);
}
