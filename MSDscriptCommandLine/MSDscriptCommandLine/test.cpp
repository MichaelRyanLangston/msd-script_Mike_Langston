//
//  test.cpp
//  MSDscriptCommandLine
//
//  Created by Michael Langston on 4/14/21.
//
// This file contains all of the tests used to test each of the functions in the program.

#include <stdio.h>
#include <sstream>
#include "catch.h"
#include "pointermgmt.h"
#include "parse.hpp"
#include "expr.hpp"
#include "val.hpp"
#include "env.hpp"
#include "step.hpp"
#include "continue.hpp"

//Parse tests
TEST_CASE("parse inner"){
    {
        std::stringstream testing ("@");
        CHECK_THROWS_WITH(parse_inner(testing), "Invalid Input...");
    }
    {
        std::stringstream testing ("_@");
        CHECK_THROWS_WITH(parse_inner(testing), "Unexpected keyword found.");
    }
    {
        std::stringstream testing ("_f@");
        CHECK_THROWS_WITH(parse_inner(testing), "Invalid Input...");
    }
    {
        std::stringstream testing ("(4");
        CHECK_THROWS_WITH(parse_inner(testing), "One or more closing parentheses are missing.");
    }
    {
        std::stringstream testing ("(4)");
        CHECK(parse_inner(testing)->equals(NEW(NumExpr)(4)));
    }
    {
        std::stringstream testing ("_ifx==5_then5_else6");
        CHECK(parse_inner(testing)->equals(NEW(IfExpr)(NEW(EqExpr)(NEW(VarExpr)("x"), NEW(NumExpr)(5)), NEW(NumExpr)(5), NEW(NumExpr)(6))));
    }
    {
        //the _ has been consumed at this point
        std::stringstream testing ("_letx=5_inx+3");
        CHECK(parse_inner(testing)->equals(NEW(LetExpr)("x", NEW(NumExpr)(5), NEW(AddExpr)(NEW(VarExpr)("x"), NEW(NumExpr) (3)))));
    }
    {
        std::stringstream testing ("_true");
        CHECK(parse_inner(testing)->equals(NEW(BoolExpr)(true)));
    }
    {
        std::stringstream testing ("_false");
        CHECK(parse_inner(testing)->equals(NEW(BoolExpr)(false)));
    }
}

TEST_CASE("parse multicand"){
    {
        std::stringstream rep_cout ("         factrl   (   factrl   )       (   10   )   ");
        CHECK(parse_multicand(rep_cout)->equals(NEW(CallExpr)(NEW(CallExpr)(NEW(VarExpr)("factrl"), NEW(VarExpr)("factrl")), NEW(NumExpr)(10))));
    }
}

TEST_CASE("parse addend"){
    {
        std::stringstream rep_cout (" 3 * 4");
        CHECK(parse_addend(rep_cout)->equals(NEW(MultExpr)(NEW(NumExpr)(3), NEW(NumExpr)(4))));
    }
    {
        std::stringstream rep_cout (" 3 + 4");
        CHECK(parse_addend(rep_cout)->equals(NEW(NumExpr)(3)));
    }
    {
        std::stringstream rep_cout (" 3 ");
        CHECK(parse_addend(rep_cout)->equals(NEW(NumExpr)(3)));
    }
    {
        //the == is ignored and just 3 is returned according the lodgic
        std::stringstream rep_cout (" 3 == 4");
        CHECK(parse_addend(rep_cout)->equals(NEW(NumExpr)(3)));
    }
}

TEST_CASE("parse_comparg"){
    {
        std::stringstream rep_cout (" 3 + 4");
        CHECK(parse_comparg(rep_cout)->equals(NEW(AddExpr)(NEW(NumExpr)(3), NEW(NumExpr)(4))));
    }
    {
        std::stringstream rep_cout (" 3 * 4");
        CHECK(parse_comparg(rep_cout)->equals(NEW(MultExpr)(NEW(NumExpr)(3), NEW(NumExpr)(4))));
    }
    {
        std::stringstream rep_cout (" 3 ");
        CHECK(parse_comparg(rep_cout)->equals(NEW(NumExpr)(3)));
    }
    {
        //the == is ignored and just 3 is returned according the present lodgic we have
        std::stringstream rep_cout (" 3 == 4");
        CHECK(parse_comparg(rep_cout)->equals(NEW(NumExpr)(3)));
    }
}

TEST_CASE("parse expr"){
    {
        std::stringstream rep_cout (" 3 == 4");
        CHECK(parse_expr(rep_cout)->equals(NEW(EqExpr)(NEW(NumExpr)(3), NEW(NumExpr)(4))));
    }
    {
        std::stringstream rep_cout (" 3 =? 4");
        CHECK_THROWS_WITH(parse_expr(rep_cout), "Invalid Operand...");
    }
    {
        std::stringstream rep_cout (" 3 + 4");
        CHECK(parse_expr(rep_cout)->equals(NEW(AddExpr)(NEW(NumExpr)(3), NEW(NumExpr)(4))));
    }
    {
        std::stringstream rep_cout (" 3 * 4");
        CHECK(parse_expr(rep_cout)->equals(NEW(MultExpr)(NEW(NumExpr)(3), NEW(NumExpr)(4))));
    }
    {
        std::stringstream rep_cout (" 3 ");
        CHECK(parse_expr(rep_cout)->equals(NEW(NumExpr)(3)));
    }
    {
        std::stringstream rep_cout ("    _let     f     =    _fun        (x)   x+    1     _in   f    (   5   )   ");
        CHECK(parse_expr(rep_cout)->equals(NEW(LetExpr)("f", NEW(FunExpr)("x",NEW(AddExpr)(NEW(VarExpr)("x"), NEW(NumExpr)(1))), NEW(CallExpr)(NEW(VarExpr)("f"), NEW(NumExpr)(5)))));
    }
    {
        std::stringstream rep_cout ("_let factrl = _fun (factrl) _fun (x) _if x == 1 _then 1 _else x * factrl(factrl)(x + -1) _in  factrl(factrl)(10)");
        PTR(EqExpr) conditional = NEW(EqExpr)(NEW(VarExpr)("x"), NEW(NumExpr)(1));
        PTR(MultExpr) multElse = NEW(MultExpr)(NEW(VarExpr)("x"), NEW(CallExpr)(NEW(CallExpr)(NEW(VarExpr)("factrl"), NEW(VarExpr)("factrl")), NEW(AddExpr)(NEW(VarExpr)("x"), NEW(NumExpr)(-1))));
        PTR(IfExpr) insideIfElse = NEW(IfExpr)(conditional, NEW(NumExpr)(1), multElse);
        PTR(FunExpr) insidefunction = NEW(FunExpr)("x", insideIfElse);
        PTR(FunExpr) function_rhs = NEW(FunExpr)("factrl", insidefunction);
        PTR(LetExpr) overAll = NEW(LetExpr)("factrl", function_rhs, NEW(CallExpr)(NEW(CallExpr)(NEW(VarExpr)("factrl"), NEW(VarExpr)("factrl")), NEW(NumExpr)(10)));
        CHECK(parse_expr(rep_cout)->equals(overAll));
    }
}

TEST_CASE("parse_fun"){

}

TEST_CASE("parse_if"){
    //No spaces
    {
        //the _ has been consumed at this point
        std::stringstream testing ("ifx==5_then5_else6");
        CHECK(parse_if(testing)->equals(NEW(IfExpr)(NEW(EqExpr)(NEW(VarExpr)("x"), NEW(NumExpr)(5)), NEW(NumExpr)(5), NEW(NumExpr)(6))));
    }
    //some spaces
    {
        std::stringstream testing ("if  \n  x   \n   ==    \n   5  \n    _then \n    5   \n   _else  \n    6");
        CHECK(parse_if(testing)->equals(NEW(IfExpr)(NEW(EqExpr)(NEW(VarExpr)("x"), NEW(NumExpr)(5)), NEW(NumExpr)(5), NEW(NumExpr)(6))));
    }
}

TEST_CASE("parse_let"){
    //No spaces
    {
        //the _ has been consumed at this point
        std::stringstream testing ("letx=5_inx+3");
        CHECK(parse_let(testing)->equals(NEW(LetExpr)("x", NEW(NumExpr)(5), NEW(AddExpr)(NEW(VarExpr)("x"), NEW(NumExpr) (3)))));
    }
    //some spaces
    {
        std::stringstream testing ("let  x  =   5   _in  x   +  3");
        CHECK(parse_let(testing)->equals(NEW(LetExpr)("x", NEW(NumExpr)(5), NEW(AddExpr)(NEW(VarExpr)("x"), NEW(NumExpr) (3)))));
    }
}

TEST_CASE("parse_var"){
    {
        std::stringstream testing ("g");
        CHECK(parse_var(testing)->equals(NEW(VarExpr)("g")));
    }

    {
        std::stringstream testing ("guess0");
        CHECK(parse_var(testing)->equals(NEW(VarExpr)("guess")));
    }

    {
        std::stringstream testing ("gu0ess");
        CHECK(parse_var(testing)->equals(NEW(VarExpr)("gu")));
    }

    {
        std::stringstream testing ("0guess");
        CHECK(parse_var(testing)->equals(NEW(VarExpr)("")));
    }
}

TEST_CASE("parse_num"){
    {
        std::stringstream testing ("33");
        CHECK(parse_num(testing)->equals(NEW(NumExpr)(33)));
    }

    {
        std::stringstream testing ("-33");
        CHECK(parse_num(testing)->equals(NEW(NumExpr)(-33)));
    }

    {
        std::stringstream testing ("--33");
        CHECK_THROWS_WITH(parse_num(testing), "parse_num: and unexpected charater has been detected between \"-\" and the number...");
    }

    {
        std::stringstream testing ("-");
        CHECK_THROWS_WITH(parse_num(testing), "parse_num: and unexpected charater has been detected between \"-\" and the number...");
    }

    {
        std::stringstream testing ("g");
        CHECK_THROWS_WITH(parse_num(testing), "parse_num: and unexpected charater has been detected between \"-\" and the number...");
    }
}

TEST_CASE("parse_keyword"){
    {
        std::stringstream testing ("_in");
        parse_keyword(testing, "_in");
        CHECK(testing.get() == EOF);
    }

    {
        std::stringstream testing ("_n");
        CHECK_THROWS_WITH(parse_keyword(testing, "_in"), "Unexpected keyword found.");
    }
}

TEST_CASE("skip_whitespace"){
    {
        std::stringstream testing ("       j");
        skip_whitespace(testing);
        CHECK(testing.get() == 'j');
    }

    {
        std::stringstream testing ("j        ");
        skip_whitespace(testing);
        CHECK(testing.get() == 'j');
    }
}

TEST_CASE("consume_character"){
    {
        std::stringstream testing ("j");
        consume_character(testing, 'j');
        CHECK(testing.get() == EOF);
    }

    {
        std::stringstream testing ("p");
        CHECK_THROWS_WITH(consume_character(testing, 'j'), "The character being consumed doesn't match the character you expected.");
    }

}


//Expr Tests

TEST_CASE("Expr"){
    CHECK((NEW(MultExpr)(NEW(AddExpr)(NEW(NumExpr)(0), NEW(VarExpr)("y")), NEW(NumExpr)(1)))->to_string() == "((0+y)*1)");
}

TEST_CASE("NumExpr"){
    /* equals() */
    CHECK((NEW(NumExpr)(1))->equals(NEW(NumExpr)(1)));
    CHECK(!((NEW(NumExpr)(1))->equals(NEW(NumExpr)(2))));
    CHECK(!((NEW(NumExpr)(1))->equals(NEW(BoolExpr)(false))));
    
    /* interp() */
    CHECK((NEW(NumExpr)(1))->interp(EmptyEnv::empty)->equals(NEW(NumVal)(1)));
    
    /* print() */
        {
            std::stringstream rep_cout ("");
            (NEW(NumExpr)(1))->print(rep_cout);
            CHECK(rep_cout.str() == "1");
        }
    
    /* pretty_print() */
//        {
//            std::stringstream rep_cout ("");
//            one->pretty_print(rep_cout);
//            CHECK(rep_cout.str() == "1");
//        }
}

TEST_CASE("AddExpr"){
    
    //Test Varialbels
        PTR(NumExpr) one = NEW(NumExpr)(1);
        PTR(NumExpr) two = NEW(NumExpr)(2);
        
        PTR(AddExpr) one_two = NEW(AddExpr)(one,two);
        PTR(AddExpr) two_one = NEW(AddExpr)(two,one);
        PTR(AddExpr) add_two_add_exper = NEW(AddExpr)(one_two, two_one);
        PTR(AddExpr) vars = NEW(AddExpr)(NEW(VarExpr)("x"), NEW(NumExpr)(3));
        PTR(AddExpr) vars2 = NEW(AddExpr)(NEW(NumExpr)(8), NEW(VarExpr)("y"));
        PTR(AddExpr) vars3 = NEW(AddExpr)(one_two, NEW(AddExpr)(NEW(VarExpr)("z"), NEW(NumExpr)(7)));
        PTR(AddExpr) vars4 = NEW(AddExpr)(NEW(AddExpr)(NEW(VarExpr)("a"), NEW(NumExpr)(3)), two_one);
        
        
        PTR(MultExpr) mult_one_two = NEW(MultExpr)(one,two);
        PTR(MultExpr) mult_two_one = NEW(MultExpr)(two,one);
        PTR(AddExpr) add_two_mult_exper = NEW(AddExpr)(mult_one_two, mult_two_one);
    
    
    //Checking Add Equality
        CHECK(one_two->equals(NEW(AddExpr)(NEW(NumExpr)(1), NEW(NumExpr)(2))));
    
    //Checking Add Inequality
        CHECK(!(one_two->equals(two_one)));
    
    //Checking Class Inequality;
        CHECK(!(one_two->equals(one)));
        CHECK(!(one_two->equals(mult_one_two)));
    
    /* interp() */
        CHECK(one_two->interp(EmptyEnv::empty)->equals(NEW(NumVal)(3)));
        CHECK(add_two_add_exper->interp(EmptyEnv::empty)->equals(NEW(NumVal)(6)));
        CHECK(add_two_mult_exper->interp(EmptyEnv::empty)->equals(NEW(NumVal)(4)));
    
    //Checking print()
        //No nesting
            {
                std::stringstream rep_cout ("");
                (NEW(AddExpr)(NEW(NumExpr)(1), NEW(VarExpr)("x")))->print(rep_cout);
                CHECK(rep_cout.str() == "(1+x)");
            }
        //Nested right
            {
                std::stringstream rep_cout("");
                (NEW(AddExpr)(NEW(NumExpr)(3),NEW(AddExpr)(NEW(NumExpr)(1), NEW(VarExpr)("x"))))->print(rep_cout);
                CHECK(rep_cout.str() == "(3+(1+x))");
            }
        //Nested left
            {
                std::stringstream rep_cout("");
                (NEW(AddExpr)(NEW(AddExpr)(NEW(NumExpr)(1), NEW(VarExpr)("x")), NEW(NumExpr)(3)))->print(rep_cout);
                CHECK(rep_cout.str() == "((1+x)+3)");
            }
    /*Depricated*/
//    //Checking pretty_print()
//        //No nesting
//            {
//                std::stringstream rep_cout ("");
//                (new Add(new Num(1), new Var("x")))->pretty_print(rep_cout);
//                CHECK(rep_cout.str() == "1 + x");
//            }
//        //Add w/ Nested right Add
//            {
//                std::stringstream rep_cout("");
//                (new Add(new Num(3),new Add(new Num(1), new Var("x"))))->pretty_print(rep_cout);
//                CHECK(rep_cout.str() == "3 + 1 + x");
//            }
//        //Add w/ Nested left Add
//            {
//                std::stringstream rep_cout("");
//                (new Add(new Add(new Num(1), new Var("x")), new Num(3)))->pretty_print(rep_cout);
//                CHECK(rep_cout.str() == "(1 + x) + 3");
//            }
//        //Add w/ Nested right Mult
//            {
//                std::stringstream rep_cout("");
//                (new Add(new Num(3), new Mult(new Num(1), new Var("x"))))->pretty_print(rep_cout);
//                CHECK(rep_cout.str() == "3 + 1 * x");
//            }
//        //Add w/ Nested left Mult
//            {
//                std::stringstream rep_cout("");
//                (new Add(new Mult(new Num(1), new Var("x")), new Num(3)))->pretty_print(rep_cout);
//                CHECK(rep_cout.str() == "1 * x + 3");
//            }
}

//Method Tests
TEST_CASE("MultExpr"){
    
    //Test Varialbels
        PTR(NumExpr) one = NEW(NumExpr)(1);
        PTR(NumExpr) two = NEW(NumExpr)(2);

        PTR(AddExpr) one_two = NEW(AddExpr)(one,two);
        PTR(AddExpr) two_one = NEW(AddExpr)(two,one);

        PTR(MultExpr) mult_one_two = NEW(MultExpr)(one,two);
        PTR(MultExpr) mult_two_one = NEW(MultExpr)(two,one);
        PTR(MultExpr) mult_two_mult_exper = NEW(MultExpr)(mult_one_two, mult_two_one);
        PTR(MultExpr) mult_two_add_exper = NEW(MultExpr)(one_two, two_one);
        PTR(MultExpr) vars = NEW(MultExpr)(NEW(VarExpr)("x"), NEW(NumExpr)(3));
        PTR(MultExpr) vars2 = NEW(MultExpr)(NEW(NumExpr)(8), NEW(VarExpr)("y"));
        PTR(MultExpr) vars3 = NEW(MultExpr)(one_two, NEW(MultExpr)(NEW(VarExpr)("z"), NEW(NumExpr)(7)));
        PTR(MultExpr) vars4 = NEW(MultExpr)(NEW(MultExpr)(NEW(VarExpr)("a"), NEW(NumExpr)(3)), two_one);
    
    
    //Checking Mult Equality
        CHECK(mult_one_two->equals(NEW(MultExpr)(NEW(NumExpr)(1), NEW(NumExpr)(2))));
    
    //Checking Mult Inequality
        CHECK(!(mult_one_two->equals(mult_two_one)));
    
    //Checking Class Inequality
        CHECK(!(mult_one_two->equals(one_two)));
        CHECK(!(mult_one_two->equals(one)));
    
    /* interp() */
        CHECK(mult_one_two->interp(EmptyEnv::empty)->equals(NEW(NumVal)(2)));
        CHECK(mult_two_add_exper->interp(EmptyEnv::empty)->equals(NEW(NumVal)(9)));
        CHECK(mult_two_mult_exper->interp(EmptyEnv::empty)->equals(NEW(NumVal)(4)));
        
    //Checking print()
        //No nesting
            {
                std::stringstream rep_cout ("");
                (NEW(MultExpr)(NEW(NumExpr)(1), NEW(VarExpr)("x")))->print(rep_cout);
                CHECK(rep_cout.str() == "(1*x)");
            }
        //Nested right
            {
                std::stringstream rep_cout("");
                (NEW(MultExpr)(NEW(NumExpr)(3),NEW(MultExpr)(NEW(NumExpr)(1), NEW(VarExpr)("x"))))->print(rep_cout);
                CHECK(rep_cout.str() == "(3*(1*x))");
            }
        //Nested left
            {
                std::stringstream rep_cout("");
                (NEW(MultExpr)(NEW(MultExpr)(NEW(NumExpr)(1), NEW(VarExpr)("x")), NEW(NumExpr)(3)))->print(rep_cout);
                CHECK(rep_cout.str() == "((1*x)*3)");
            }
    
    /*Depricated*/
//    //Checking pretty_print()
//        //No nesting
//            {
//                std::stringstream rep_cout ("");
//                (new Mult(new Num(1), new Var("x")))->pretty_print(rep_cout);
//                CHECK(rep_cout.str() == "1 * x");
//            }
//        //Mult w/ Nested right Add
//            {
//                std::stringstream rep_cout("");
//                (new Mult(new Num(3),new Add(new Num(1), new Var("x"))))->pretty_print(rep_cout);
//                CHECK(rep_cout.str() == "3 * (1 + x)");
//            }
//        //Mult w/ Nested left Add
//            {
//                std::stringstream rep_cout("");
//                (new Mult(new Add(new Num(1), new Var("x")), new Num(3)))->pretty_print(rep_cout);
//                CHECK(rep_cout.str() == "(1 + x) * 3");
//            }
//        //Mult w/ Nested right Mult
//            {
//                std::stringstream rep_cout("");
//                (new Mult(new Num(3), new Mult(new Num(1), new Var("x"))))->pretty_print(rep_cout);
//                CHECK(rep_cout.str() == "3 * 1 * x");
//            }
//        //Mult w/ Nested left Mult
//            {
//                std::stringstream rep_cout("");
//                (new Mult(new Mult(new Num(1), new Var("x")), new Num(3)))->pretty_print(rep_cout);
//                CHECK(rep_cout.str() == "(1 * x) * 3");
//            }
//        //Mult w/ Nested left Mult
//            {
//                std::stringstream rep_cout("");
//                (new Mult(new Mult(new Num(1), new Var("x")), new Add(new Var("y"), new Num(5))))->pretty_print(rep_cout);
//                CHECK(rep_cout.str() == "(1 * x) * (y + 5)");
//            }
}

//Method Tests
TEST_CASE("VarExpr"){

    //Test Varialbels
        PTR(NumExpr) one = NEW(NumExpr)(1);
        PTR(NumExpr) two = NEW(NumExpr)(2);
        PTR(AddExpr) one_two = NEW(AddExpr)(one,two);
        PTR(MultExpr) mult_one_two = NEW(MultExpr)(one,two);
        PTR(VarExpr) first = NEW(VarExpr)("first");
        PTR(VarExpr) second = NEW(VarExpr)("second");

    //Checking Var Equality
        CHECK(first->equals(NEW(VarExpr)("first")));

    //Checking Var Inequality
        CHECK(!(first->equals(second)));
    
    //Checking Class Inequality
        CHECK(!(first->equals(one)));
        CHECK(!(first->equals(one_two)));
        CHECK(!(first->equals(mult_one_two)));
    
    /* interp() */
    CHECK_THROWS_WITH((NEW(VarExpr)("x"))->interp(EmptyEnv::empty), "free variable detected: x");
    CHECK((NEW(VarExpr)("x"))->interp(NEW(ExtendedEnv)("x", NEW(NumVal)(4), EmptyEnv::empty))->equals(NEW(NumVal)(4)));
    
    //Checking print()
        {
            std::stringstream rep_cout ("");
            (NEW(VarExpr)("x"))->print(rep_cout);
            CHECK(rep_cout.str() == "x");
        }
    
    /*Depricated*/
    //Checking pretty_print()
//        {
//            std::stringstream rep_cout ("");
//            (new Var("x"))->pretty_print(rep_cout);
//            CHECK(rep_cout.str() == "x");
//        }
}

//Method Tests
TEST_CASE("LetExpr"){
    PTR(LetExpr) firstExpression = NEW(LetExpr)("x", NEW(NumExpr)(7), NEW(MultExpr)(NEW(VarExpr)("x"), NEW(NumExpr)(7)));
    PTR(LetExpr) secondExpression = NEW(LetExpr)("x", NEW(NumExpr)(7), NEW(LetExpr)("x", NEW(NumExpr)(8), NEW(AddExpr)(NEW(VarExpr)("x"), NEW(NumExpr)(7))));
    PTR(LetExpr) thridExpression = NEW(LetExpr)("x", NEW(NumExpr)(7), NEW(LetExpr)("x", NEW(AddExpr)(NEW(VarExpr)("x"), NEW(NumExpr)(8)), NEW(AddExpr)(NEW(VarExpr)("x"), NEW(NumExpr)(7))));
    PTR(LetExpr) fourthExpression = NEW(LetExpr)("x", NEW(NumExpr)(7), NEW(LetExpr)("y", NEW(NumExpr)(8), NEW(AddExpr)(NEW(VarExpr)("x"), NEW(NumExpr)(7))));
    PTR(LetExpr) invalidExpression_body_free_var = NEW(LetExpr)("x", NEW(NumExpr)(7), NEW(LetExpr)("x", NEW(AddExpr)(NEW(VarExpr)("x"), NEW(NumExpr)(8)), NEW(AddExpr)(NEW(VarExpr)("y"), NEW(NumExpr)(7))));
    PTR(LetExpr) invalidExpression_rhs_free_var = NEW(LetExpr)("y", NEW(VarExpr)("y"), NEW(AddExpr)(NEW(NumExpr)(3), NEW(VarExpr)("y")));
    PTR(LetExpr) noVariablePresent =NEW(LetExpr)("x", NEW(NumExpr)(7), NEW(MultExpr)(NEW(NumExpr)(5), NEW(NumExpr)(7)));
    
    //print variables
    PTR(LetExpr) print_test = NEW(LetExpr)("x", NEW(NumExpr)(5), NEW(AddExpr)(NEW(LetExpr)("y", NEW(NumExpr)(3), NEW(AddExpr)(NEW(VarExpr)("y"), NEW(NumExpr)(2))), NEW(VarExpr)("x")));
    
    //Checking _let Equality
    CHECK(firstExpression->equals(NEW(LetExpr)("x", NEW(NumExpr)(7), NEW(MultExpr)(NEW(VarExpr)("x"), NEW(NumExpr)(7)))));

    //Checking _let Inequality
    CHECK(!(firstExpression->equals(NEW(LetExpr)("y", NEW(NumExpr)(7), NEW(MultExpr)(NEW(VarExpr)("x"), NEW(NumExpr)(7))))));
    CHECK(!(firstExpression->equals(NEW(LetExpr)("x", NEW(NumExpr)(8), NEW(MultExpr)(NEW(VarExpr)("x"), NEW(NumExpr)(7))))));
    CHECK(!(firstExpression->equals(NEW(LetExpr)("x", NEW(NumExpr)(7), NEW(MultExpr)(NEW(VarExpr)("y"), NEW(NumExpr)(7))))));
    CHECK(!(firstExpression->equals(NEW(LetExpr)("x", NEW(NumExpr)(7), NEW(MultExpr)(NEW(VarExpr)("x"), NEW(NumExpr)(8))))));
    CHECK(!(firstExpression->equals(NEW(LetExpr)("x", NEW(NumExpr)(7), NEW(MultExpr)(NEW(NumExpr)(7), NEW(VarExpr)("x"))))));
    
    //Checking Class Inequality
    CHECK(!firstExpression->equals(NEW(VarExpr)("x")));
    CHECK(!firstExpression->equals(NEW(NumExpr)(7)));
    CHECK(!firstExpression->equals(NEW(AddExpr)(NEW(NumExpr)(0), NEW(NumExpr)(0))));
    CHECK(!firstExpression->equals(NEW(MultExpr)(NEW(NumExpr)(0), NEW(NumExpr)(0))));
    
    
    //Checking interp()
    CHECK(firstExpression->interp(EmptyEnv::empty)->equals(NEW(NumVal)(49)));
    CHECK(secondExpression->interp(EmptyEnv::empty)->equals(NEW(NumVal)(15)));
    CHECK(thridExpression->interp(EmptyEnv::empty)->equals(NEW(NumVal)(22)));
    CHECK(fourthExpression->interp(EmptyEnv::empty)->equals(NEW(NumVal)(14)));
    CHECK(noVariablePresent->interp(EmptyEnv::empty)->equals(NEW(NumVal)(35)));
    CHECK_THROWS_WITH(invalidExpression_rhs_free_var->interp(EmptyEnv::empty), "free variable detected: y");
    CHECK_THROWS_WITH(invalidExpression_body_free_var->interp(EmptyEnv::empty), "free variable detected: y");
    
    //Checking print()
    CHECK(firstExpression->to_string() == "(_let x=7 _in (x*7))");
    CHECK(print_test->to_string() == "(_let x=5 _in ((_let y=3 _in (y+2))+x))");
    
    /*Depricated*/
//    //Checking pretty_print()
//    Add* exampleTests = new Add(new Mult(new Num(5), new _let("x", new Num(5), new Var("x"))), new Num(1));
//    {
//        std::stringstream rep_cout ("");
//        print_test->pretty_print(rep_cout);
//        CHECK(rep_cout.str() == "_let x = 5\n_in  (_let y = 3\n      _in  y + 2) + x");
//    }
}

TEST_CASE("BoolExpr"){
    /* equals() */
    CHECK((NEW(BoolExpr)(false))->equals(NEW(BoolExpr)(false)));
    CHECK(!((NEW(BoolExpr)(false))->equals(NEW(BoolExpr)(true))));
    CHECK(!((NEW(BoolExpr)(false))->equals(NEW(NumExpr)(4))));
    
    /* interp() */
    CHECK((NEW(BoolExpr)(true))->interp(EmptyEnv::empty)->equals(NEW(BoolVal)(true)));
    
    /* print */
    {
        std::stringstream rep_cout ("");
        (NEW(BoolExpr)(true))->print(rep_cout);
        CHECK(rep_cout.str() == "_true");
    }
    {
        std::stringstream rep_cout ("");
        (NEW(BoolExpr)(false))->print(rep_cout);
        CHECK(rep_cout.str() == "_false");
    }
}

TEST_CASE("EqExpr"){
    /* equals() */
    CHECK((NEW(EqExpr)(NEW(NumExpr)(0), NEW(NumExpr)(1)))->equals(NEW(EqExpr)(NEW(NumExpr)(0), NEW(NumExpr)(1))));
    CHECK(!((NEW(EqExpr)(NEW(NumExpr)(0), NEW(NumExpr)(0)))->equals(NEW(EqExpr)(NEW(NumExpr)(0), NEW(NumExpr)(1)))));
    CHECK(!((NEW(EqExpr)(NEW(NumExpr)(1), NEW(NumExpr)(1)))->equals(NEW(EqExpr)(NEW(NumExpr)(0), NEW(NumExpr)(1)))));
    CHECK(!((NEW(EqExpr)(NEW(NumExpr)(1), NEW(NumExpr)(1)))->equals(NEW(NumExpr)(0))));
    
    /* interp() */
    CHECK((NEW(EqExpr)(NEW(BoolExpr)(false), NEW(BoolExpr)(false)))->interp(EmptyEnv::empty)->equals(NEW(BoolVal)(true)));
    CHECK((NEW(EqExpr)(NEW(BoolExpr)(false), NEW(BoolExpr)(true)))->interp(EmptyEnv::empty)->equals(NEW(BoolVal)(false)));
    CHECK((NEW(EqExpr)(NEW(NumExpr)(1), NEW(BoolExpr)(true)))->interp(EmptyEnv::empty)->equals(NEW(BoolVal)(false)));
    CHECK((NEW(EqExpr)(NEW(BoolExpr)(false), NEW(NumExpr)(0)))->interp(EmptyEnv::empty)->equals(NEW(BoolVal)(false)));
    
    /* print() */
    {
        std::stringstream rep_cout ("");
        (NEW(EqExpr)(NEW(VarExpr)("x"), NEW(NumExpr)(1)))->print(rep_cout);
        CHECK(rep_cout.str() == "(x==1)");
    }
    {
        std::stringstream rep_cout ("");
        (NEW(EqExpr)(NEW(VarExpr)("7"), NEW(BoolExpr)(false)))->print(rep_cout);
        CHECK(rep_cout.str() == "(7==_false)");
    }
}

TEST_CASE("IfExpr"){
    /* equals() */
    CHECK((NEW(IfExpr)(NEW(NumExpr)(0), NEW(NumExpr)(0) , NEW(NumExpr)(0)))->equals(NEW(IfExpr)(NEW(NumExpr)(0), NEW(NumExpr)(0) , NEW(NumExpr)(0))));
    CHECK(!((NEW(IfExpr)(NEW(NumExpr)(0), NEW(NumExpr)(0) , NEW(NumExpr)(0)))->equals(NEW(NumExpr)(0))));
    CHECK(!((NEW(IfExpr)(NEW(NumExpr)(1), NEW(NumExpr)(0) , NEW(NumExpr)(0)))->equals(NEW(IfExpr)(NEW(NumExpr)(0), NEW(NumExpr)(0) , NEW(NumExpr)(0)))));
    CHECK(!((NEW(IfExpr)(NEW(NumExpr)(0), NEW(NumExpr)(1) , NEW(NumExpr)(0)))->equals(NEW(IfExpr)(NEW(NumExpr)(0), NEW(NumExpr)(0) , NEW(NumExpr)(0)))));
    CHECK(!((NEW(IfExpr)(NEW(NumExpr)(0), NEW(NumExpr)(0) , NEW(NumExpr)(1)))->equals(NEW(IfExpr)(NEW(NumExpr)(0), NEW(NumExpr)(0) , NEW(NumExpr)(0)))));
    CHECK(!((NEW(IfExpr)(NEW(VarExpr)("x"), NEW(NumExpr)(0) , NEW(NumExpr)(0)))->equals(NEW(IfExpr)(NEW(NumExpr)(0), NEW(NumExpr)(0) , NEW(NumExpr)(0)))));
    CHECK(!((NEW(IfExpr)(NEW(NumExpr)(0), NEW(VarExpr)("x") , NEW(NumExpr)(0)))->equals(NEW(IfExpr)(NEW(NumExpr)(0), NEW(NumExpr)(0) , NEW(NumExpr)(0)))));
    CHECK(!((NEW(IfExpr)(NEW(NumExpr)(0), NEW(NumExpr)(0) , NEW(VarExpr)("x")))->equals(NEW(IfExpr)(NEW(NumExpr)(0), NEW(NumExpr)(0) , NEW(NumExpr)(0)))));
    
    /* interp() */
    CHECK((NEW(IfExpr)(NEW(BoolExpr)(true), NEW(NumExpr)(1), NEW(NumExpr)(0)))->interp(EmptyEnv::empty)->equals(NEW(NumVal)(1)));
    CHECK((NEW(IfExpr)(NEW(BoolExpr)(false), NEW(NumExpr)(1), NEW(NumExpr)(0)))->interp(EmptyEnv::empty)->equals(NEW(NumVal)(0)));
    
    /* print() */
    {
        std::stringstream rep_cout ("");
        (NEW(IfExpr)(NEW(NumExpr)(2), NEW(NumExpr)(0) , NEW(VarExpr)("x")))->print(rep_cout);
        CHECK(rep_cout.str() == "(_if 2_then 0_else x)");
    }
    {
        std::stringstream rep_cout ("");
        (NEW(IfExpr)(NEW(NumExpr)(5), NEW(NumExpr)(6) , NEW(NumExpr)(4)))->print(rep_cout);
        CHECK(rep_cout.str() == "(_if 5_then 6_else 4)");
    }
}

TEST_CASE("FunExpr"){
    /* equals() */
    CHECK((NEW(FunExpr)("x", NEW(AddExpr)(NEW(VarExpr)("x"), NEW(NumExpr)(4))))->equals(NEW(FunExpr)("x", NEW(AddExpr)(NEW(VarExpr)("x"), NEW(NumExpr)(4)))));
    CHECK(!((NEW(FunExpr)("x", NEW(AddExpr)(NEW(VarExpr)("x"), NEW(NumExpr)(4))))->equals(NEW(FunExpr)("y", NEW(AddExpr)(NEW(VarExpr)("x"), NEW(NumExpr)(4))))));
    CHECK(!((NEW(FunExpr)("x", NEW(AddExpr)(NEW(VarExpr)("x"), NEW(NumExpr)(4))))->equals(NEW(FunExpr)("x", NEW(AddExpr)(NEW(VarExpr)("z"), NEW(NumExpr)(4))))));
    CHECK(!((NEW(FunExpr)("x", NEW(AddExpr)(NEW(VarExpr)("x"), NEW(NumExpr)(4))))->equals(NEW(BoolExpr)(true))));
    
    /* interp() */
    CHECK((NEW(FunExpr)("x", NEW(AddExpr)(NEW(VarExpr)("x"), NEW(NumExpr)(4))))->interp(EmptyEnv::empty)->equals(NEW(FunVal)("x", NEW(AddExpr)(NEW(VarExpr)("x"), NEW(NumExpr)(4)), EmptyEnv::empty)));
    
    /* print() */
    {
        std::stringstream rep_cout ("");
        (NEW(FunExpr)("x", NEW(AddExpr)(NEW(VarExpr)("x"), NEW(NumExpr)(4))))->print(rep_cout);
        CHECK(rep_cout.str() == "(_fun (x) (x+4))");
    }
}

TEST_CASE("CallExpr Tests"){
    /* equals() */
    CHECK((NEW(CallExpr)(NEW(VarExpr)("x"), NEW(MultExpr)(NEW(VarExpr)("x"), NEW(NumExpr)(4))))->equals(NEW(CallExpr)(NEW(VarExpr)("x"), NEW(MultExpr)(NEW(VarExpr)("x"), NEW(NumExpr)(4)))));
    CHECK(!((NEW(CallExpr)(NEW(VarExpr)("x"), NEW(MultExpr)(NEW(VarExpr)("x"), NEW(NumExpr)(4))))->equals(NEW(CallExpr)(NEW(VarExpr)("y"), NEW(MultExpr)(NEW(VarExpr)("x"), NEW(NumExpr)(4))))));
    CHECK(!((NEW(CallExpr)(NEW(VarExpr)("x"), NEW(MultExpr)(NEW(VarExpr)("x"), NEW(NumExpr)(4))))->equals(NEW(CallExpr)(NEW(VarExpr)("x"), NEW(MultExpr)(NEW(VarExpr)("y"), NEW(NumExpr)(4))))));
    CHECK(!((NEW(CallExpr)(NEW(VarExpr)("x"), NEW(MultExpr)(NEW(VarExpr)("x"), NEW(NumExpr)(4))))->equals(NEW(CallExpr)(NEW(BoolExpr)(true), NEW(MultExpr)(NEW(VarExpr)("x"), NEW(NumExpr)(4))))));
    CHECK(!((NEW(CallExpr)(NEW(VarExpr)("x"), NEW(MultExpr)(NEW(VarExpr)("x"), NEW(NumExpr)(4))))->equals(NEW(BoolExpr)(true))));
    
    /* interp() */
    CHECK((NEW(CallExpr)(NEW(FunExpr)("x", NEW(MultExpr)(NEW(VarExpr)("x"), NEW(NumExpr)(4))), NEW(NumExpr)(4)))->interp(EmptyEnv::empty)->equals(NEW(NumVal)(16)));
    
    // (_fun(x) (_fun(x) x + 1)(2) + x)(5)
    CHECK((NEW(CallExpr)(NEW(FunExpr)("x", NEW(AddExpr)(NEW(CallExpr)(NEW(FunExpr)("x", NEW(AddExpr)(NEW(VarExpr)("x"),NEW(NumExpr)(1))),NEW(NumExpr)(2)), NEW(VarExpr)("x"))), NEW(NumExpr)(5)))->interp(EmptyEnv::empty));
    
    /* print() */
    {
        std::stringstream rep_cout ("");
        (NEW(CallExpr)(NEW(VarExpr)("f"), NEW(NumExpr)(4)))->print(rep_cout);
        CHECK(rep_cout.str() == "f(4)");
    }
    
}

//Val Tests

TEST_CASE("NumVal"){
    
    /* equals() */
    CHECK((NEW(NumVal)(0))->equals(NEW(NumVal)(0)));
    CHECK(!((NEW(NumVal)(1))->equals(NEW(NumVal)(2))));
    CHECK(!((NEW(NumVal)(2))->equals(NEW(BoolVal)(true))));
    
    /* add_to() */
    CHECK((NEW(NumVal)(4))->add_to(NEW(NumVal)(1))->equals(NEW(NumVal)(5)));
    CHECK_THROWS_WITH((NEW(NumVal)(4))->add_to(NEW(BoolVal)(true)), "Non-NumVal object detected. Cannot perform addition.");
    
    /* mult_by() */
    CHECK((NEW(NumVal)(4))->mult_by(NEW(NumVal)(1))->equals(NEW(NumVal)(4)));
    CHECK_THROWS_WITH((NEW(NumVal)(4))->mult_by(NEW(BoolVal)(true)), "Non-NumVal object detected. Cannot perform multiplication.");
    
    /* is_true() */
    CHECK_THROWS_WITH((NEW(NumVal)(4))->is_true(), "NumVal object detected. Cannot check for _true condition.");
    
    /* call() */
    CHECK_THROWS_WITH((NEW(NumVal)(4))->call(NEW(NumVal)(4)), "NumVal object detected. Cannot perform a call.");
}

TEST_CASE("BoolVal"){
    /* equals() */
    CHECK((NEW(BoolVal)(false))->equals(NEW(BoolVal)(false)));
    CHECK(!((NEW(BoolVal)(true))->equals(NEW(BoolVal)(false))));
    CHECK(!((NEW(BoolVal)(true))->equals(NEW(NumVal)(0))));
    
    /* add_to() */
    CHECK_THROWS_WITH((NEW(BoolVal)(true))->add_to(NEW(NumVal)(false)), "BoolVal object detected. Cannot perform addition.");
    
    /* mult_by() */
    CHECK_THROWS_WITH((NEW(BoolVal)(true))->mult_by(NEW(NumVal)(false)),"BoolVal object detected. Cannot perform multiplication.");
    
    /* is_true() */
    CHECK((NEW(BoolVal)(true))->is_true());
    CHECK(!((NEW(BoolVal)(false))->is_true()));
    
    /* call() */
    CHECK_THROWS_WITH((NEW(BoolVal)(true))->call(NEW(BoolVal)(true)), "BoolVal object detected. Cannot perform a call.");
}

TEST_CASE("FunVal"){
    /* equals() */
    CHECK((NEW(FunVal)("x", NEW(AddExpr)(NEW(VarExpr)("x"), NEW(NumExpr)(4)), EmptyEnv::empty))->equals(NEW(FunVal)("x", NEW(AddExpr)(NEW(VarExpr)("x"), NEW(NumExpr)(4)), EmptyEnv::empty)));
    CHECK(!((NEW(FunVal)("x", NEW(AddExpr)(NEW(VarExpr)("x"), NEW(NumExpr)(4)), EmptyEnv::empty))->equals(NEW(FunVal)("y", NEW(AddExpr)(NEW(VarExpr)("x"), NEW(NumExpr)(4)), EmptyEnv::empty))));
    CHECK(!((NEW(FunVal)("x", NEW(AddExpr)(NEW(VarExpr)("x"), NEW(NumExpr)(4)), EmptyEnv::empty))->equals(NEW(FunVal)("x", NEW(MultExpr)(NEW(VarExpr)("x"), NEW(NumExpr)(4)), EmptyEnv::empty))));
    CHECK(!((NEW(FunVal)("x", NEW(AddExpr)(NEW(VarExpr)("x"), NEW(NumExpr)(4)), EmptyEnv::empty))->equals(NEW(BoolVal)(true))));
    
    /* add_to() */
    CHECK_THROWS_WITH((NEW(FunVal)("x", NEW(AddExpr)(NEW(VarExpr)("x"), NEW(NumExpr)(4)), EmptyEnv::empty))->add_to(NEW(BoolVal)(true)), "FunVal object detected. Cannot perform addition.");
    
    /* mult_by() */
    CHECK_THROWS_WITH((NEW(FunVal)("x", NEW(AddExpr)(NEW(VarExpr)("x"), NEW(NumExpr)(4)), EmptyEnv::empty))->mult_by(NEW(BoolVal)(true)), "FunVal object detected. Cannot perform multiplication.");
    /* is_true() */
    CHECK_THROWS_WITH((NEW(FunVal)("x", NEW(AddExpr)(NEW(VarExpr)("x"), NEW(NumExpr)(4)), EmptyEnv::empty))->is_true(), "FunVal object detected. Cannot check for _true condition.");
    
    /* call() */
    CHECK((NEW(FunVal)("x", NEW(AddExpr)(NEW(VarExpr)("x"), NEW(NumExpr)(4)), EmptyEnv::empty))->call(NEW(NumVal)(4))->equals(NEW(NumVal)(8)));
    CHECK_THROWS_WITH((NEW(FunVal)("x", NEW(AddExpr)(NEW(VarExpr)("x"), NEW(NumExpr)(4)), EmptyEnv::empty))->call(NEW(BoolVal)(true)), "BoolVal object detected. Cannot perform addition.");
    CHECK((NEW(FunVal)("x", NEW(AddExpr)(NEW(VarExpr)("x"), NEW(NumExpr)(4)), EmptyEnv::empty))->call((NEW(FunVal)("x", NEW(AddExpr)(NEW(VarExpr)("x"), NEW(NumExpr)(4)), EmptyEnv::empty))->call(NEW(NumVal)(3)))->equals(NEW(NumVal)(11)));
}

//Env Tests
//none

//Step Tests

TEST_CASE("Interp_by_steps Testing"){
    //NumExper Step Testing
    std::stringstream testNum("1");
    CHECK(Step::interp_by_steps(parse_expr(testNum))->equals(NEW(NumVal)(1)));
    
    //BoolExper Step Testing
    std::stringstream testBoolTrue("_true");
    CHECK(Step::interp_by_steps(parse_expr(testBoolTrue))->equals(NEW(BoolVal)(true)));
    
    std::stringstream testBoolFalse("_false");
    CHECK(Step::interp_by_steps(parse_expr(testBoolFalse))->equals(NEW(BoolVal)(false)));
    
    //FunExper / LetExper / CallExper Step Testing
    std::stringstream testFun("_let f = _fun (x) x + 1 _in f(5)");
    CHECK(Step::interp_by_steps(parse_expr(testFun))->equals(NEW(NumVal)(6)));
    
    //IfExper Testing
    std::stringstream testIfTrue("_if 1 == 1 _then 4 _else 5");
    CHECK(Step::interp_by_steps(parse_expr(testIfTrue))->equals(NEW(NumVal)(4)));
    
    std::stringstream testIfFalse("_if 1 == 2 _then 4 _else 5");
    CHECK(Step::interp_by_steps(parse_expr(testIfFalse))->equals(NEW(NumVal)(5)));
    
    //AddExper StepTesting
    std::stringstream test1("1 + 2");
    CHECK(Step::interp_by_steps(parse_expr(test1))->equals(NEW(NumVal)(3)));
    
    //MultExper Step Testing
    std::stringstream test2("1 * 2");
    CHECK(Step::interp_by_steps(parse_expr(test2))->equals(NEW(NumVal)(2)));
    
    //EqExper Step Testing
    std::stringstream testEqFalse("1 == 2");
    CHECK(Step::interp_by_steps(parse_expr(testEqFalse))->equals(NEW(BoolVal)(false)));
    
    std::stringstream testEqTrue("1 == 1");
    CHECK(Step::interp_by_steps(parse_expr(testEqTrue))->equals(NEW(BoolVal)(true)));
    
    //Overall Step Testing
    std::stringstream testFactorial("_let factrl = _fun (factrl) _fun (x) _if x == 1 _then 1 _else x * factrl(factrl)(x + -1) _in  factrl(factrl)(10)");
    CHECK(Step::interp_by_steps(parse_expr(testFactorial))->equals(NEW(NumVal)(3628800)));
    
}

//Continue Tests
//None
