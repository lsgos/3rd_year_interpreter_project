#include "primitives.h"

SExp *primitive_cons(std::list<SExp *> args, Env &env) {
  if (args.size() != 2) {
    throw evaluation_error("Incorrect number of arguments in primitive cons");
  }
  // evaluate argument list
  std::for_each(args.begin(), args.end(), [&](SExp *&a) { a = a->eval(env); });
  // SExp*& is a reference to an SExp pointer

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

SExp *primitive_car(std::list<SExp *> args, Env &env) {
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

SExp *primitive_isnull(std::list<SExp *> args, Env &env) {
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

SExp *primitive_cdr(std::list<SExp *> args, Env &env) {
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

SExp *primitive_quote(std::list<SExp *> args, Env &env) {
  if (args.size() != 1) {
    throw evaluation_error("Incorrect number of arguments in primitive quote");
  }
  SExp *atom = args.front();
  return atom;
}

SExp *primitive_define(std::list<SExp *> args, Env &env) {
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

SExp *primitive_lambda(std::list<SExp *> args, Env &env) {
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
                               "expected "
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
SExp *primitive_if(std::list<SExp *> args, Env &env) {
  if (args.size() != 3) {
    throw evaluation_error("Incorrect number of arguments in if special form");
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

SExp *primitive_exit(std::list<SExp *> args, Env &env) {
  throw exit_interpreter();
  return nullptr;
}

SExp *primitive_numeric_eq(std::list<SExp *> args, Env &env) {
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
SExp *primitive_eq(std::list<SExp *> args, Env &env) {
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
  std::for_each(args.begin(), args.end(), [&](SExp *&a) { a = a->eval(env); });

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
    result =
        (static_cast<Bool *>(arg1)->val() == static_cast<Bool *>(arg2)->val());
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

SExp *primitive_eval(std::list<SExp *> args, Env &env) {
  // evaluate the argument as a lisp expression

  if (args.size() != 1) {
    throw evaluation_error(
        "Incorrect number of arguments in function eval: expected 1");
  }
  // evaluate the value of the argument as as a lisp expression.
  SExp *exp = args.front();
  exp = exp->eval(env);
  return exp->eval(env);
}

SExp *primitive_is_number(std::list<SExp *> args, Env &env) {
  // test if the input is a number
  if (args.size() != 1) {
    throw evaluation_error("Incorrect number of arguments in function number?");
  }
  bool result = false;
  if (args.front()->type() == LispType::Number) {
    result = true;
  }
  return env.allocate(new Bool(result));
}

SExp *primitive_open_output_port(std::list<SExp *> args, Env &env) {
  if (args.size() != 1) {
    throw evaluation_error(
        "Invalid number of arguments in function open-output-port");
  }
  SExp *fname = args.front()->eval(env);
  if (fname->type() != LispType::String) {
    throw evaluation_error(
        "Invalid argument to function open-output-port: expected string");
  }
  std::string name = static_cast<String *>(fname)->val();

  try {
    return env.allocate(new OutPort(name));
  } catch (io_error e) {
    // use booleans to signal errors to the calling program, since we aren't
    // going to implement exception catching
    return env.allocate(new Bool(false));
  }
}

SExp *primitive_display(std::list<SExp *> args, Env &env) {
  SExp *msg, *output_port;
  switch (args.size()) {
  case 1:
    msg = args.front()->eval(env);
    output_port = env.lookup("std-output-port");
    break;
  case 2:
    msg = args.front()->eval(env);
    args.pop_front();
    output_port = args.front()->eval(env);
    break;
  default:
    throw evaluation_error(
        "Invalid number of arguments to builtin define: expected 1 or 2");
  }
  if (output_port->type() != LispType::OutPort) {
    std::cout << int(output_port->type());
    throw evaluation_error(
        "Cannot write to a non-port type: expected output-port");
  }
  // write the string representation of the object to the output port
  std::stringstream buf;
  auto repr = DisplayRepresentor(buf);
  msg->exec(repr);

  static_cast<OutPort *>(output_port)->write(buf.str(), env);
  return env.lookup("null");
}
SExp *primitive_displayln(std::list<SExp *> args, Env &env) {
  SExp *msg, *output_port;
  switch (args.size()) {
  case 1:
    msg = args.front()->eval(env);
    output_port = env.lookup("std-output-port");
    break;
  case 2:
    msg = args.front()->eval(env);
    ;
    args.pop_front();
    output_port = args.front()->eval(env);
    break;
  default:
    throw evaluation_error(
        "Invalid number of arguments to builtin define: expected 1 or 2");
  }
  if (output_port->type() != LispType::OutPort) {
    std::cout << int(output_port->type());
    throw evaluation_error(
        "Cannot write to a non-port type: expected output-port");
  }
  // write the string representation of the object to the output port
  std::stringstream buf;
  auto repr = DisplayRepresentor(buf);
  msg->exec(repr);

  static_cast<OutPort *>(output_port)->write(buf.str(), env);
  static_cast<OutPort *>(output_port)->write("\n", env);

  return env.lookup("null");
}
SExp *primitive_close_output_port(std::list<SExp *> args, Env &env) {
  if (args.size() != 1) {
    throw evaluation_error(
        "Incorrect number of arguments in function close-output-port");
  }
  SExp *arg = args.front()->eval(env);
  if (arg->type() != LispType::OutPort) {
    throw evaluation_error(
        "Invalid call of close-outport-port on non output port type");
  }
  static_cast<OutPort *>(arg)->close();
  return env.lookup("null");
}

SExp *primitive_modulo(std::list<SExp *> args, Env &env) {
  if (args.size() != 2) {
    throw evaluation_error("Invalid number of arguments in function %");
  }

  for (auto it = args.begin(); it != args.end(); ++it) {
    (*it) = (*it)->eval(env);
    if ((*it)->type() != LispType::Number) {
      throw evaluation_error("Encountered non-numeric arguments in function %");
    }
  }
  double argument = static_cast<Number *>(args.front())->val();
  args.pop_front();
  double mod = static_cast<Number *>(args.front())->val();
  double result = std::fmod(argument, mod);
  return env.allocate(new Number(result));
}

SExp *primitive_not(std::list<SExp *> args, Env &env) {
  if (args.size() != 1) {
    throw evaluation_error("Incorrect number of arguments in function not");
  }
  auto x = args.front()->eval(env);
  bool result = !is_true(x);
  return env.allocate(new Bool(result));
}

// Here, we implement the common higher order functions map, filter and fold.
// Many loops can be expressed as some combination of these
// three functions, and by utilitising the c++ algorithms we can avoid using
// explicit recursion in our language to a certain extent

// this is a helper function for the three higher order functions below: they
// need to
// pass in quoted variables as though they were defined in lisp (where they woud
// pass in
// atoms that would get evaled in the functions scope.) We simulate this by
// constructing a
// new quote variable

SExp *quote_var(SExp *var, Env &env) {
  return env.allocate(new List(std::list<SExp *>{env.lookup("quote"), var}));
}

// (map f xs) where xs = (a b c d ...) --> ((f a) (f b) (f c) (f d) ...)
SExp *primitive_map(std::list<SExp *> args, Env &env) {
  if (args.size() != 2) {
    throw evaluation_error("Incorrect number of arguments in primitive map");
  }
  // evaluate arguments
  std::for_each(args.begin(), args.end(), [&](SExp *&a) { a = a->eval(env); });

  if (!is_function(args.front()->type())) {
    throw evaluation_error(
        "Illegal first argument in function map: expected function");
  }
  Function *func = static_cast<Function *>(args.front());
  args.pop_front();
  if (args.front()->type() != LispType::List) {
    throw evaluation_error(
        "Illegal second argument in function map: expected list");
  }
  List *list = static_cast<List *>(args.front());

  std::list<SExp *> elements = list->elems;
  // apply the function func to every element in the list
  std::for_each(elements.begin(), elements.end(), [&func, &env](SExp *&a) {
    a = func->call(std::list<SExp *>{quote_var(a, env)}, env);
  });
  return env.allocate(new List(elements));
}

SExp *primitive_filter(std::list<SExp *> args, Env &env) {
  if (args.size() != 2) {
    throw evaluation_error("Incorrect number of arguments in primitive filter");
  }
  // evaluate arguments
  std::for_each(args.begin(), args.end(), [&](SExp *&a) { a = a->eval(env); });

  if (!is_function(args.front()->type())) {
    throw evaluation_error(
        "Illegal first argument in function filter: expected function");
  }
  Function *pred = static_cast<Function *>(args.front());
  args.pop_front();
  if (args.front()->type() != LispType::List) {
    throw evaluation_error(
        "Illegal second argument in function filter: expected list");
  }
  List *list = static_cast<List *>(args.front());

  std::list<SExp *> elements = list->elems;

  // remove all elements from the list for which the predicate pred does not
  // return true, using the lispy critereon for truthiness
  // std::remove_if is slightly tricky: it moves all passing elements to the
  // start of the list, and returns an iterator to the last element
  // that does not pass the predicate, requiring the manual erasing of those
  // elements

  elements.erase(
      std::remove_if(elements.begin(), elements.end(),
                     [&pred, &env](SExp *&a) -> bool {
                       return !is_true(pred->call(
                           std::list<SExp *>{quote_var(a, env)}, env));
                     }),
      elements.end());

  return env.allocate(new List(elements));
}

// Implements a left fold over the list with the last element as the accumulator

//(fold (f acc x -> acc) acc (xs) )
SExp *primitive_fold(std::list<SExp *> args, Env &env) {
  if (args.size() != 3) {
    throw evaluation_error("Incorrect number of arguments in primitive fold");
  }
  // evaluate arguments
  std::for_each(args.begin(), args.end(), [&](SExp *&a) { a = a->eval(env); });

  if (!is_function(args.front()->type())) {
    throw evaluation_error(
        "Illegal first argument in function fold: expected function");
  }
  Function *func = static_cast<Function *>(args.front());
  args.pop_front();

  SExp *init = args.front(); // the initial accumulator can be of any type
  args.pop_front();

  if (args.front()->type() != LispType::List) {
    throw evaluation_error(
        "Illegal second argument in function fold: expected list");
  }
  List *list = static_cast<List *>(args.front());

  std::list<SExp *> elements = list->elems;

  // perform a fold over the elements
  SExp *result = std::accumulate(
      elements.begin(), elements.end(), init,
      [&env, &func](SExp *acc, SExp *elem) -> SExp * {
        return func->call(
            std::list<SExp *>{quote_var(acc, env), quote_var(elem, env)}, env);
      });

  return result;
}

SExp *primitive_list(std::list<SExp *> args, Env &env) {
  // construct a list from elems: this is very simple!
  std::for_each(args.begin(), args.end(), [&](SExp *&a) { a = a->eval(env); });
  return env.allocate(new List(args));
}