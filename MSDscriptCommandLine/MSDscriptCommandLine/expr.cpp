//
//  expr.cpp
//  MSDscriptCommandLine
//
//  Created by Michael Langston on 1/24/21.
//

#include "expr.hpp"


//        _let factrl = _fun (factrl)
//                        _fun (x)
//                          _if x == 1
//                          _then 1
//                          _else x * factrl(factrl)(x + -1)
//        _in  factrl(factrl)(10)


/*Exper Interface Implemntations*/
//Methods
std::string Expr::to_string(){
    std::stringstream stringify("");
    this->print(stringify);
    return stringify.str();
}

/* Depricated */
//void Expr::pretty_print(std::ostream& out){
//    pretty_print_at(out, print_group_none, 0);
//}

//Method Tests
TEST_CASE("Exper"){
    CHECK((NEW(MultExpr)(NEW(AddExpr)(NEW(NumExpr)(0), NEW(VarExpr)("y")), NEW(NumExpr)(1)))->to_string() == "((0+y)*1)");
}

/*Num Implementation*/
//Default Constructor
NumExpr::NumExpr(int rep){
    this->rep = rep;
}

//Methods
bool NumExpr::equals(PTR(Expr)e){
    PTR(NumExpr)ne = CAST(NumExpr)(e);
    if (ne == NULL)
        return false;
    else
        return this->rep == ne->rep;
}

PTR(Val) NumExpr::interp(PTR(Env) env){
    return NEW(NumVal)(this->rep);
}

void NumExpr::step_interp(){
    Step::mode = Step::continue_mode;
    Step::val = NEW(NumVal) (this->rep);
    Step::cont = Step::cont;
}

void NumExpr::print(std::ostream& out){
    out << this->rep;
}

//void Num::pretty_print_at(std::ostream& out, print_mode_t mode, int num){
//    print(out);
//}

//Method Tests
TEST_CASE("Num"){
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


/*Add implementations*/
//Default Constructor
AddExpr::AddExpr(PTR(Expr)lhs, PTR(Expr)rhs) {
    this->lhs = lhs;
    this->rhs = rhs;
}

//Methods
bool AddExpr::equals(PTR(Expr)e){
    PTR(AddExpr)ae = CAST(AddExpr)(e);
    if (ae == NULL)
        return false;
    else
        return (this->lhs->equals(ae->lhs) && this->rhs->equals(ae->rhs));
}

PTR(Val) AddExpr::interp(PTR(Env) env){
    return lhs->interp(env)->add_to(rhs->interp(env));
}

void AddExpr::step_interp(){
    Step::mode = Step::interp_mode;
    Step::expr = lhs;
    Step::env = Step::env;
    Step::cont = NEW(RightThenAddCont) (rhs, Step::env, Step::cont);
}

void AddExpr::print(std::ostream& out){
    out << "(";
    this->lhs->print(out);
    out << "+";
    this->rhs->print(out);
    out << ")";
}

//void Add::pretty_print_at(std::ostream& out, print_mode_t mode, int num){
//    if(mode == print_group_none){
//        this->lhs->pretty_print_at(out, print_group_add, 0);
//        out << " + ";
//        this->rhs->pretty_print_at(out, print_group_none, 0);
//    }
//    else {
//        out << "(";
//        this->lhs->pretty_print_at(out, print_group_add, 0);
//        out << " + ";
//        this->rhs->pretty_print_at(out, print_group_none, 0);
//        out << ")";
//    }
//}

//Method Tests
TEST_CASE("Add"){
    
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






/*Mult implementations*/
//Default Constructor
MultExpr::MultExpr(PTR(Expr)lhs, PTR(Expr)rhs) {
    this->lhs = lhs;
    this->rhs = rhs;
}

//Methods
bool MultExpr::equals(PTR(Expr)e){
    PTR(MultExpr) me = CAST(MultExpr)(e);
    if (me == NULL)
        return false;
    else
        return (this->lhs->equals(me->lhs) && this->rhs->equals(me->rhs));
}

PTR(Val) MultExpr::interp(PTR(Env) env){
    return this->lhs->interp(env)->mult_by(this->rhs->interp(env));
}

void MultExpr::step_interp(){
    Step::mode = Step::interp_mode;
    Step::expr = lhs;
    Step::env = Step::env;
    Step::cont = NEW(RightThenMultCont) (rhs, Step::env, Step::cont);
}

void MultExpr::print(std::ostream& out){
    out << "(";
    this->lhs->print(out);
    out << "*";
    this->rhs->print(out);
    out << ")";
}

/*Depricated*/
//void Mult::pretty_print_at(std::ostream& out, print_mode_t mode, int num){
//    if(mode == print_group_add_or_mult){
//        out << "(";
//        this->lhs->pretty_print_at(out, print_group_none, 0);
//        out << " * ";
//        this->rhs->pretty_print_at(out, print_group_none, 0);
//        out << ")";
//    }
//    else {
//        this->lhs->pretty_print_at(out, print_group_add_or_mult, 0);
//        out << " * ";
//        this->rhs->pretty_print_at(out, print_group_add, 0);
//    }
//}

//Method Tests
TEST_CASE("Mult Test"){
    
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

/*Var implementations*/
//Default Constructor
VarExpr::VarExpr(std::string var){
    this->var = var;
}

//Methods
bool VarExpr::equals(PTR(Expr)e){
    PTR(VarExpr)v = CAST(VarExpr)(e);
    if(v == NULL)
        return false;
    else
        return this->var == v->var;
}

PTR(Val) VarExpr::interp(PTR(Env) env){
    return env->lookup(var);
}

void VarExpr::step_interp(){
    Step::mode = Step::continue_mode;
    Step::val = Step::env->lookup(var);
    Step::cont = Step::cont;
}

void VarExpr::print(std::ostream& out){
    out << this->var;
}


/*Depricated*/
//void Var::pretty_print_at(std::ostream& out, print_mode_t mode, int num){
//    print(out);
//}

//Method Tests
TEST_CASE("Var"){

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






/*LetExpr implementations*/
//Default Constructor
LetExpr::LetExpr(std::string lhs_name, PTR(Expr)rhs,PTR(Expr)body){
    this->lhs_name = lhs_name;
    this->rhs = rhs;
    this->body = body;
}

//Methods
bool LetExpr::equals(PTR(Expr)e){
    PTR(LetExpr)l = CAST(LetExpr)(e);
    if(l == NULL)
        return false;
    else
        return this->lhs_name == l->lhs_name && this->rhs->equals(l->rhs) && this->body->equals(l->body);
}

PTR(Val) LetExpr::interp(PTR(Env) env){
    return body->interp(NEW(ExtendedEnv)(lhs_name, rhs->interp(env), env));
}

void LetExpr::step_interp(){
    Step::mode = Step::interp_mode;
    Step::expr = rhs;
    Step::env = Step::env;
    Step::cont = NEW(LetBodyCont) (lhs_name, body, Step::env, Step::cont);
}

void LetExpr::print(std::ostream& out){
    out << "(_let " << this->lhs_name << "=";
    this->rhs->print(out);
    out << " _in ";
    this->body->print(out);
    out << ")";
}

/*Depricated*/
//void _let::pretty_print_at(std::ostream& out, print_mode_t mode, int num){
//    out << "_let " << this->lhs_name << " = ";
//    this->rhs->pretty_print_at(out, print_group_none, 0);
//    out << "\n" << "_in ";
//    this->body->pretty_print_at(out, print_group_add, 0);
//}

//Method Tests
TEST_CASE("_let"){
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





/* BoolExpr Implementation */
//Default Constructor
BoolExpr::BoolExpr(bool rep){
    this->rep = rep;
}

//Methods
bool BoolExpr::equals(PTR(Expr)e){
    PTR(BoolExpr)be = CAST(BoolExpr)(e);
    if(be == NULL)
        return false;
    else
        return this->rep == be->rep;
}

PTR(Val) BoolExpr::interp(PTR(Env) env){
    return NEW(BoolVal)(this->rep);
}

void BoolExpr::step_interp(){
    Step::mode = Step::continue_mode;
    Step::val = NEW(BoolVal)(this->rep);
    Step::cont = Step::cont;
}

void BoolExpr::print(std::ostream& out){
    if (this->rep)
        out << "_true";
    else
        out << "_false";
}


TEST_CASE("BoolExpr Tests"){
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





/* EqExpr Implementation */
//Default Constructor
EqExpr::EqExpr(PTR(Expr) lhs, PTR(Expr) rhs){
    this->lhs = lhs;
    this->rhs = rhs;
}

//Methods
bool EqExpr::equals(PTR(Expr)e){
    PTR(EqExpr) ee = CAST(EqExpr)(e);
    if (ee == NULL)
        return false;
    else
        return (this->lhs->equals(ee->lhs) && this->rhs->equals(ee->rhs));
}

PTR(Val) EqExpr::interp(PTR(Env) env){
    if (this->lhs->interp(env)->equals(this->rhs->interp(env)))
        return NEW(BoolVal)(true);
    else
        return NEW(BoolVal)(false);
}

void EqExpr::step_interp(){
    Step::mode = Step::interp_mode;
    Step::expr = lhs;
    Step::env = Step::env;
    Step::cont = NEW(RightThenEqCont) (rhs, Step::env, Step::cont);
}

void EqExpr::print(std::ostream& out){
    out << "(";
    this->lhs->print(out);
    out << "==";
    this->rhs->print(out);
    out << ")";
}

TEST_CASE("EqExpr Tests"){
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

/* IfExpr Implementation */
//Default Constructor
IfExpr::IfExpr(PTR(Expr) comparison, PTR(Expr) if_true, PTR(Expr) if_false){
    this->comparison = comparison;
    this->if_true = if_true;
    this->if_false = if_false;
}

//Methods
bool IfExpr::equals(PTR(Expr)e){
    PTR(IfExpr) ife = CAST(IfExpr)(e);
    if (ife == NULL)
        return false;
    else
        return this->comparison->equals(ife->comparison) && this->if_true->equals(ife->if_true) && this->if_false->equals(ife->if_false);
}

PTR(Val) IfExpr::interp(PTR(Env) env){
    if(this->comparison->interp(env)->is_true())
        return this->if_true->interp(env);
    else
        return this->if_false->interp(env);
}

void IfExpr::step_interp(){
    Step::mode = Step::interp_mode;
    Step::expr = this->comparison;
    Step::env = Step::env;
    Step::cont = NEW(IfBranchCont) (this->if_true, this->if_false, Step::env, Step::cont);
}

void IfExpr::print(std::ostream& out){
    out << "(_if ";
    this->comparison->print(out);
    out << "_then ";
    this->if_true->print(out);
    out << "_else ";
    this->if_false->print(out);
    out << ")";
    
}


TEST_CASE("IfExpr Tests"){
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

/* FunExpr Implementations */
//Default Constructor
FunExpr::FunExpr(std::string formal_arg, PTR(Expr) body){
    this->formal_arg = formal_arg;
    this->body = body;
}

//Methods
bool FunExpr::equals(PTR(Expr) e){
    PTR(FunExpr) fe = CAST(FunExpr)(e);
    if (fe == NULL)
        return false;
    else
        return this->formal_arg == fe->formal_arg && this->body->equals(fe->body);
}

PTR(Val) FunExpr::interp(PTR(Env) env){
    return NEW(FunVal)(formal_arg, body, env);
}

void FunExpr::step_interp(){
    Step::mode = Step::continue_mode;
    Step::val = NEW(FunVal)(formal_arg, body, Step::env);
    Step::cont = Step::cont;
}

void FunExpr::print(std::ostream& out){
    out << "(_fun (" + this->formal_arg + ") ";
    this->body->print(out);
    out << ")";
}

TEST_CASE("FunExpr Tests"){
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

/* CallExpr Implementations */
//Default Constructor
CallExpr::CallExpr(PTR(Expr) to_be_called, PTR(Expr) actual_arg){
    this->to_be_called = to_be_called;
    this->actual_arg = actual_arg;
}

//Methods
bool CallExpr::equals(PTR(Expr)e){
    PTR(CallExpr) ce = CAST(CallExpr)(e);
    if (ce == NULL)
        return false;
    else
        return this->to_be_called->equals(ce->to_be_called) && this->actual_arg->equals(ce->actual_arg);
}

PTR(Val) CallExpr::interp(PTR(Env) env){
    return to_be_called->interp(env)->call(actual_arg->interp(env));
}

void CallExpr::step_interp(){
    Step::mode = Step::interp_mode;
    Step::expr = to_be_called;
    Step::cont = NEW(ArgThenCallCont) (actual_arg, Step::env, Step::cont);
}

void CallExpr::print(std::ostream& out){
    this->to_be_called->print(out);
    out << "(";
    this->actual_arg->print(out);
    out << ")";
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
