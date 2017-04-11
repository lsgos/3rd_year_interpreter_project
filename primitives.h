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
identical function signiture.

In the env.cc class, these are all used to construct PrimitiveFunction objects,
which are then
bound to the global scope in order to make these functions avaliable in user
programs.

*/

// All these functions have essentially the same signitures: is there a way to
// express this?

namespace primitive {
SExp *cons(std::list<SExp *> args, Env &env);
SExp *car(std::list<SExp *> args, Env &env);
SExp *isnull(std::list<SExp *> args, Env &env);
SExp *cdr(std::list<SExp *> args, Env &env);
SExp *quote(std::list<SExp *> args, Env &env);
SExp *define(std::list<SExp *> args, Env &env);
SExp *lambda(std::list<SExp *> args, Env &env);
SExp *if_stmt(std::list<SExp *> args, Env &env);
SExp *not_stmt(std::list<SExp *> args, Env &env);
SExp *exit_stmt(std::list<SExp *> args, Env &env);
SExp *numeric_eq(std::list<SExp *> args, Env &env);
SExp *eq(std::list<SExp *> args, Env &env);
SExp *modulo(std::list<SExp *> args, Env &env);
SExp *eval(std::list<SExp *> args, Env &env);
SExp *is_number(std::list<SExp *> args, Env &env);
SExp *open_output_port(std::list<SExp *> args, Env &env);
SExp *close_output_port(std::list<SExp *> args, Env &env);
SExp *open_input_port(std::list<SExp *> args, Env &env);
SExp *close_input_port(std::list<SExp *> args, Env &env);
SExp *port_to_string(std::list<SExp *> args, Env &env);
SExp *display(std::list<SExp *> args, Env &env);
SExp *displayln(std::list<SExp *> args, Env &env);
SExp *map(std::list<SExp *> args, Env &env);
SExp *filter(std::list<SExp *> args, Env &env);
SExp *fold(std::list<SExp *> args, Env &env);
SExp *list(std::list<SExp *> args, Env &env);
SExp *logical_and(std::list<SExp *> args, Env &env);
SExp *logical_or(std::list<SExp *> args, Env &env);
SExp *read(std::list<SExp *> args, Env &env);
}
#endif
