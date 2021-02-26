//
//  val.cpp
//  MSDscriptCommandLine
//
//  Created by Michael Langston on 2/25/21.
//

#include "val.hpp"

/*Consturctor*/
NumVal::NumVal(int value){
    this->value = value;
}

/*Methods*/
int NumVal::getVal(){
    return this->value;
}

bool NumVal::equals(Val* v){
    NumVal* nv = dynamic_cast<NumVal*>(v);
    if (nv == NULL)
        return false;
    else
        return this->value == nv->value;
}
