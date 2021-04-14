//
//  expr.cpp
//  MSDscriptCommandLine
//
//  Created by Michael Langston on 1/24/21.
//

#include "expr.hpp"

/*Expr Implemntation*/
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
