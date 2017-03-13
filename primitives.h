#ifndef PRIMITIVES_H
#define PRIMITIVES_H

#include "env.h"
#include "sexp.h"
#include <algorithm>
#include <cmath>
#include <sstream>
/*
This is where most of the language builtin functions and special forms are
defined.
They ought to have fairly self-explanitory functionality. Note they all have an
identical
function signiture.

In the env.cc class, these are all used to construct PrimitiveFunction objects,
which are then
bound to the global scope in order to make these functions avaliable in user
programs.

*/

// All these functions have essentially the same signitures: is there a way to
// express this?
SExp *primitive_cons(std::list<SExp *> args, Env &env);
SExp *primitive_car(std::list<SExp *> args, Env &env);
SExp *primitive_isnull(std::list<SExp *> args, Env &env);
SExp *primitive_cdr(std::list<SExp *> args, Env &env);
SExp *primitive_quote(std::list<SExp *> args, Env &env);
SExp *primitive_define(std::list<SExp *> args, Env &env);
SExp *primitive_lambda(std::list<SExp *> args, Env &env);
SExp *primitive_if(std::list<SExp *> args, Env &env);
SExp *primitive_not(std::list<SExp *> args, Env &env);
SExp *primitive_exit(std::list<SExp *> args, Env &env);
SExp *primitive_numeric_eq(std::list<SExp *> args, Env &env);
SExp *primitive_eq(std::list<SExp *> args, Env &env);
SExp *primitive_modulo(std::list<SExp *> args, Env &env);
SExp *primitive_eval(std::list<SExp *> args, Env &env);
SExp *primitive_is_number(std::list<SExp *> args, Env &env);
SExp *primitive_open_output_port(std::list<SExp *> args, Env &env);
SExp *primitive_close_output_port(std::list<SExp *> args, Env &env);
SExp *primitive_display(std::list<SExp *> args, Env &env);
SExp *primitive_displayln(std::list<SExp *> args, Env &env);
SExp *primitive_map(std::list<SExp *> args, Env &env);
SExp *primitive_filter(std::list<SExp *> args, Env &env);
SExp *primitive_fold(std::list<SExp *> args, Env &env);
#endif
