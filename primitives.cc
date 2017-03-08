/*
This is where the builtin language primitives are defined, such as car,
cdr, etc. It's just a
*/
#include "primitives.h"

SExp* primitive_cons(std::list<SExp*> args, Env& env) {
    if (args.size() != 2) {
      throw evaluation_error("Incorrect number of arguments in primitive cons");
    }
    for (auto it = args.begin(); it != args.end(); ++it) {
      (*it) = (*it)->eval(env);
    }
    SExp *car = args.front();
    args.pop_front();
    SExp *cdr = args.front();
    // technically this is different from classical lisp as we don't
    // have a
    // dotted-list type (which I don't see as particularly useful)
    if (cdr->type() != LispType::List) {
      throw evaluation_error("Cannot cons onto a non-list "
                             "type [expected: (cons T "
                             "list)]");
    }
    // push the car argument onto the list of the second, then
    // return the
    // second
    // argument.
    // Copy the list: this only copies the pointers, so it should be
    // relatively
    // cheap
    auto list_cdr = static_cast<List *>(cdr);
    std::list<SExp *> cons_list = list_cdr->elems;
    cons_list.push_front(car);
    return env.allocate(new List(cons_list));
}


SExp* primitive_car(std::list<SExp*> args, Env& env) {
    if (args.size() != 1) {
      throw evaluation_error("Incorrect number of arguments in primitive car");
    }
    SExp *ls = args.front();
    ls = ls->eval(env);
    if (ls->type() != LispType::List) {
      throw evaluation_error("Cannot ask for the car of a non-list");
    }
    auto elems = static_cast<List *>(ls)->elems;
    if (elems.empty()) {
      throw evaluation_error("Cannot ask for the car of an empty list");
    }
    auto car = elems.front();
    return car;
}

SExp* primitive_isnull(std::list<SExp *> args, Env &env) {
    bool result = false;
    if (args.size() != 1) {
      throw evaluation_error("Incorrect number of arguments in functino null?");
    } else {
      SExp *obj = args.front()->eval(env);
      if (obj->type() == LispType::List &&
          static_cast<List *>(obj)->elems.empty()) {
        result = true;
      }
      return env.allocate(new Bool(result));
    }
}


SExp* primitive_cdr (std::list<SExp *> args, Env &env) {
    if (args.size() != 1) {
      throw evaluation_error("Incorrect number of arguments in primitive cdr");
    }
    SExp *ls = args.front();
    ls = ls->eval(env);
    if (ls->type() != LispType::List) {
      throw evaluation_error("Cannot ask for the cdr of a non list type");
    }
    auto elems = static_cast<List *>(ls)->elems; // copy list of object
    if (elems.empty()) {
      throw evaluation_error("Cannot ask for the cdr of a empty list");
    }
    elems.pop_front();
    return env.allocate(new List(elems));
}

SExp* primitive_quote(std::list<SExp *> args, Env &env) {
    if (args.size() != 1) {
      throw evaluation_error(
          "Incorrect number of arguments in primitive quote");
    }
    SExp *atom = args.front();
    return atom;
}


SExp* primitive_define (std::list<SExp *> args, Env &env) {
    if (args.size() != 2) {
      throw evaluation_error("Incorrect number of arguments in primitive "
                             "define: expected two");
    }
    auto atom = args.front();
    args.pop_front();
    if (atom->type() != LispType::Atom) {
      throw evaluation_error("Expected atomic symbol as "
                             "first argument to define");
    }
    std::string id = static_cast<Atom *>(atom)->get_identifier();
    auto value = args.front();
    value = value->eval(env);
    env.def(id, value);
    return env.allocate(new List);
}

SExp* primitive_lambda (std::list<SExp *> args, Env &env) {
    // lambda is a special form: evaluate the first argument, but
    // none of
    // the
    // others.
    if (args.size() < 2) {
      throw evaluation_error("Too few arguments in call to lambda");
    }
    // first argument to lambda must be either a list of atoms or a
    // single
    // atom
    SExp *first = args.front();
    args.pop_front();
    LispType param_type = first->type();
    // this will be used to construct the lambda
    std::list<std::string> param_list;
    if (param_type == LispType::Atom) {
      auto at = static_cast<Atom *>(first);
      param_list.push_back(at->get_identifier());
    } else if (param_type == LispType::List) {
      // check f it's a list of atoms
      auto list = static_cast<List *>(first);
      for (auto it = list->elems.begin(); it != list->elems.end(); ++it) {
        if ((*it)->type() == LispType::Atom) {
          auto at = static_cast<Atom *>(*it);
          param_list.push_back(at->get_identifier());
        } else {
          throw evaluation_error("Error in arguments to lambda: "
                                 "excepted "
                                 "identifier or list of "
                                 "identifiers");
        }
      }
    } else {
      // not valid:
      throw evaluation_error("Error in first argument to lambda: expected "
                             "identifier or list of identifiers");
    }
    // capture scopemm
    Env closure = env.capture_scope();
    return env.allocate(new LambdaFunction(closure, param_list, args));
}
  // implement the if special form
SExp* primitive_if (std::list<SExp *> args, Env &env) {
    if (args.size() != 3) {
      throw evaluation_error(
          "Incorrect number of arguments in if special form");
    }
    auto predicate = args.front();
    args.pop_front();
    auto then_clause = args.front();
    args.pop_front();
    auto else_clause = args.front();
    // evaluate the predicate expressions
    predicate = predicate->eval(env);
    if (is_true(predicate)) {
      return then_clause->eval(env);
    } else {
      return else_clause->eval(env);
    }
}

SExp* primitive_exit (std::list<SExp *> args, Env &env) {
    throw exit_interpreter();
    return nullptr;
}

SExp* primitive_numeric_eq (std::list<SExp *> args, Env &env) {
    if (args.size() < 2) {
      throw evaluation_error("Too few arguments in primitive "
                             "=; expected two or more");
    }
    bool result = true;
    SExp *first = args.front()->eval(env);
    if (first->type() != LispType::Number) {
      throw evaluation_error("Found non numeric arguments in function =");
    }
    double last = static_cast<Number *>(first)->val();
    for (auto it = args.begin()++; it != args.end(); ++it) {
      (*it) = (*it)->eval(env);
      if ((*it)->type() != LispType::Number) {
        throw evaluation_error("Found non numeric "
                               "arguments in function "
                               "=");
      }
      if (last != static_cast<Number *>(*it)->val()) {
        result = false;
        break;
      }
    }
    return env.allocate(new Bool(result));
}
SExp* primitive_eq (std::list<SExp *> args, Env &env) {
    // This is slightly different from the canonical lisp eq, which
    // compares for
    // pointer equality. This is in fact equivalent to schemes eqv.
    // However in my
    // implementation comparing for pointer equality is more or less
    // pointless as very
    // few objects will have equal pointers, so we may as well just
    // have the obvious
    // test for equality on primitives.

    if (args.size() != 2) {
      throw evaluation_error("Incorrect number of arguments "
                             "to eq?: expected two");
    }
    // eval args
    for (auto it = args.begin(); it != args.end(); ++it) {
      (*it) = (*it)->eval(env);
    }

    bool result;

    auto arg1 = args.front();
    args.pop_front();
    auto arg2 = args.front();
    auto type1 = arg1->type();
    auto type2 = arg2->type();
    if (type1 != type2) {
      result = false;
    }
    switch (type1) {
    case LispType::Number:
      result = (static_cast<Number *>(arg1)->val() ==
                static_cast<Number *>(arg2)->val());
      break;
    case LispType::String:
      result = (static_cast<String *>(arg1)->val() ==
                static_cast<String *>(arg2)->val());
      break;
    case LispType::Bool:
      result = (static_cast<Bool *>(arg1)->val() ==
                static_cast<Bool *>(arg2)->val());
      break;
    case LispType::PrimitiveFunction:
      result = (arg1 == arg2);
      break;
    default:
      // all other types are compound: these are covered by
      // equal?, so just compare pointer equality
      result = (arg1 == arg2);
    }
    return env.allocate(new Bool(result));
}
