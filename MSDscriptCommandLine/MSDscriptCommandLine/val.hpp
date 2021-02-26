//
//  val.hpp
//  MSDscriptCommandLine
//
//  Created by Michael Langston on 2/25/21.
//

#ifndef val_hpp
#define val_hpp

#include <stdio.h>

class Val{
public:
    /*Method*/
    virtual int getVal() = 0;
    virtual bool equals(Val *v) = 0;
    
};

class NumVal : public Val{
public:
    int value;
    
    /*Constructor*/
    NumVal(int value);
    
    /*Method*/
    int getVal();
    bool equals(Val *v);
};

#endif /* val_hpp */
