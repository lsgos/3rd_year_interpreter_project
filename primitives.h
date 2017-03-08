#ifndef PRIMITIVES_H
#define PRIMITIVES_H

#include "env.h"
#include "sexp.h"
SExp* primitive_cons(std::list<SExp*> args, Env& env);
SExp* primitive_car(std::list<SExp*> args, Env& env);
SExp* primitive_isnull(std::list<SExp *> args, Env &env);
SExp* primitive_cdr (std::list<SExp *> args, Env &env);
SExp* primitive_quote(std::list<SExp *> args, Env &env);
SExp* primitive_define (std::list<SExp *> args, Env &env);
SExp* primitive_lambda (std::list<SExp *> args, Env &env);
SExp* primitive_if (std::list<SExp *> args, Env &env);
SExp* primitive_exit (std::list<SExp *> args, Env &env);
SExp* primitive_numeric_eq (std::list<SExp *> args, Env &env);
SExp* primitive_eq (std::list<SExp *> args, Env &env);

#endif
