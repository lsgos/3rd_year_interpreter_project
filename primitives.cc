#include "parser.h"
#include "primitives.h"
#include <algorithm>
#include <typeinfo>

SExp *primitive::cons(std::list<SExp *> args, Env &env) {
  if (args.size() != 2) {
    throw evaluation_error("Incorrect number of arguments in primitive cons");
  }
  // evaluate argument list
  std::for_each(args.begin(), args.end(), [&](SExp *&a) { a = a->eval(env); });
  // SExp*& is a reference to an SExp pointer

  SExp *car = args.front();
  args.pop_front();
  SExp *cdr = args.front();

  List *lp = dynamic_cast<List *>(cdr);

  if (!lp) {
    throw evaluation_error("Cannot cons onto a non-list "
                           "type [expected: (cons T "
                           "list)]");
  }

  std::list<SExp *> cons_list = lp->elems;
  cons_list.push_front(car);
  return env.manage(new List(cons_list));
}

SExp *primitive::car(std::list<SExp *> args, Env &env) {
  if (args.size() != 1) {
    throw evaluation_error("Incorrect number of arguments in primitive car");
  }
  SExp *arg = args.front();
  arg = arg->eval(env);

  List *lp = dynamic_cast<List *>(arg);
  if (!lp) {
    throw evaluation_error("Cannot ask for the car of a non-list");
  }
  auto elems = lp->elems;
  if (elems.empty()) {
    throw evaluation_error("Cannot ask for the car of an empty list");
  }
  auto car = elems.front();
  return car;
}

SExp *primitive::isnull(std::list<SExp *> args, Env &env) {
  bool result = false;
  if (args.size() != 1) {
    throw evaluation_error("Incorrect number of arguments in functino null?");
  } else {
    SExp *obj = args.front()->eval(env);
    List *lp = dynamic_cast<List *>(obj);

    if (lp && lp->elems.empty()) {
      result = true;
    }
    return env.manage(new Bool(result));
  }
}

SExp *primitive::cdr(std::list<SExp *> args, Env &env) {
  if (args.size() != 1) {
    throw evaluation_error("Incorrect number of arguments in primitive cdr");
  }
  SExp *obj = args.front()->eval(env);
  List *lp = dynamic_cast<List *>(obj);
  if (!lp) {
    throw evaluation_error("Cannot ask for the cdr of a non list type");
  }
  auto elems = lp->elems; // copy list of object
  if (elems.empty()) {
    throw evaluation_error("Cannot ask for the cdr of a empty list");
  }
  elems.pop_front();
  return env.manage(new List(elems));
}

SExp *primitive::quote(std::list<SExp *> args, Env &env) {
  if (args.size() != 1) {
    throw evaluation_error("Incorrect number of arguments in primitive quote");
  }
  SExp *atom = args.front();
  return atom;
}

SExp *primitive::define(std::list<SExp *> args, Env &env) {
  if (args.size() != 2) {
    throw evaluation_error("Incorrect number of arguments in primitive "
                           "define: expected two");
  }
  Atom *ap = dynamic_cast<Atom *>(args.front());
  args.pop_front();
  if (!ap) {
    throw evaluation_error("Expected atomic symbol as "
                           "first argument to define");
  }
  std::string id = ap->get_identifier();
  auto value = args.front();
  value = value->eval(env);
  env.def(id, value);
  return env.manage(new List);
}

SExp *primitive::lambda(std::list<SExp *> args, Env &env) {

  if (args.size() < 2) {
    throw evaluation_error("Too few arguments in call to lambda");
  }

  List *list = dynamic_cast<List *>(args.front());
  args.pop_front();

  std::list<std::string> param_list;

  if (list) {

    // check f it's a list of atoms
    for (auto it = list->elems.begin(); it != list->elems.end(); ++it) {
      Atom *atp = dynamic_cast<Atom *>(*it);
      if (atp) {
        param_list.push_back(atp->get_identifier());

      } else {
        throw evaluation_error("Error in arguments to lambda: "
                               "expected "
                               "identifier or list of "
                               "identifiers");
      }
    }
  } else {
    throw evaluation_error("Error in first argument to lambda: expected "
                           "list of identifiers");
  }

  Env closure = env.capture_scope();
  return env.manage(new LambdaFunction(closure, param_list, args));
}
// implement the if special form
SExp *primitive::if_stmt(std::list<SExp *> args, Env &env) {
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

SExp *primitive::exit_stmt(std::list<SExp *> args, Env &env) {
  throw exit_interpreter();
  return nullptr;
}

SExp *primitive::numeric_eq(std::list<SExp *> args, Env &env) {
  if (args.size() < 2) {
    throw evaluation_error("Too few arguments in primitive "
                           "=; expected two or more");
  }
  bool result = true;
  SExp *first = args.front()->eval(env);
  Number *np = dynamic_cast<Number *>(first);
  if (!np) {
    throw evaluation_error("Found non numeric arguments in function =");
  }
  double comp = np->val();

  for (auto it = args.begin()++; it != args.end(); ++it) {

    np = dynamic_cast<Number *>((*it)->eval(env));
    if (!np) {
      throw evaluation_error("Found non numeric "
                             "arguments in function "
                             "=");
    }
    if (comp != np->val()) {
      result = false;
      break;
    }
  }
  return env.manage(new Bool(result));
}
SExp *primitive::eq(std::list<SExp *> args, Env &env) {
  // This is slightly different from the canonical lisp eq, which
  // compares for pointer equality. This is actually more like
  // the function eqv, which is more sensible

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
  // here we need to use the hash for the type rather than the type_info class
  // itself because the type_info class has no public copy/move constructors
  auto type1 = typeid(*arg1).hash_code();
  auto type2 = typeid(*arg2).hash_code();

  if (type1 != type2) {
    result = false;
  } else if (type1 == typeid(Number).hash_code()) {
    result = (static_cast<Number *>(arg1)->val() ==
              static_cast<Number *>(arg2)->val());
  } else if (type1 == typeid(String).hash_code()) {
    result = (static_cast<String *>(arg1)->val() ==
              static_cast<String *>(arg2)->val());
  } else if (type1 == typeid(Bool).hash_code()) {
    result =
        (static_cast<Bool *>(arg1)->val() == static_cast<Bool *>(arg2)->val());
  } else {
    result = (arg1 == arg2);
  }

  return env.manage(new Bool(result));
}

SExp *primitive::eval(std::list<SExp *> args, Env &env) {
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

SExp *primitive::is_number(std::list<SExp *> args, Env &env) {
  // test if the input is a number
  if (args.size() != 1) {
    throw evaluation_error("Incorrect number of arguments in function number?");
  }
  bool result = false;
  SExp *arg = args.front();
  if (typeid(*arg) == typeid(Number)) {
    result = true;
  }
  return env.manage(new Bool(result));
}

SExp *primitive::open_output_port(std::list<SExp *> args, Env &env) {
  if (args.size() != 1) {
    throw evaluation_error(
        "Invalid number of arguments in function open-output-port");
  }
  SExp *fname = args.front()->eval(env);
  String *sp = dynamic_cast<String *>(fname);
  if (!sp) {
    throw evaluation_error(
        "Invalid argument to function open-output-port: expected string");
  }
  std::string name = sp->val();

  try {
    return env.manage(new OutPort(name));
  } catch (io_error e) {
    // use booleans to signal errors to the calling program, since we aren't
    // going to implement exception catching
    return env.manage(new Bool(false));
  }
}

SExp *primitive::open_input_port(std::list<SExp *> args, Env &env) {
  if (args.size() != 1) {
    throw evaluation_error(
        "Invalid number of arguments in function open-output-port");
  }
  SExp *fname = args.front()->eval(env);
  String *sp = dynamic_cast<String *>(fname);

  if (!sp) {
    throw evaluation_error(
        "Invalid argument to function open-output-port: expected string");
  }
  std::string name = sp->val();

  try {
    return env.manage(new InPort(name));
  } catch (io_error e) {
    // use booleans to signal errors to the calling program, since we aren't
    // going to implement exception catching
    return env.manage(new Bool(false));
  }
}

SExp *primitive::close_input_port(std::list<SExp *> args, Env &env) {
  if (args.size() != 1) {
    throw evaluation_error(
        "Incorrect number of arguments in function close-output-port");
  }
  SExp *arg = args.front()->eval(env);
  InPort *ip = dynamic_cast<InPort *>(arg);

  if (!ip) {
    throw evaluation_error(
        "Invalid call of close-outport-port on non output port type");
  }
  ip->close();
  return env.lookup("null");
}

// read the entire contents of a file into a string
SExp *primitive::port_to_string(std::list<SExp *> args, Env &env) {
  if (args.size() != 1) {
    throw evaluation_error(
        "Incorrect number of arguments in function port->string");
  }
  SExp *arg = args.front()->eval(env);
  InPort *ip = dynamic_cast<InPort *>(arg);
  if (!ip) {
    throw evaluation_error(
        "Type error: expected a port in function port->string");
  }
  try {
    return ip->read(env);
  } catch (io_error e) {
    return env.manage(new Bool(false)); // return false if nothing can be read
  }
}

SExp *primitive::display(std::list<SExp *> args, Env &env) {
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
  OutPort *op = dynamic_cast<OutPort *>(output_port);
  if (!op) {
    throw evaluation_error(
        "Cannot write to a non-port type: expected output-port");
  }
  // write the string representation of the object to the output port
  std::stringstream buf;
  auto repr = DisplayRepresentor(buf);
  msg->exec(repr);

  op->write(buf.str(), env);
  return env.lookup("null");
}
SExp *primitive::displayln(std::list<SExp *> args, Env &env) {
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
  OutPort *op = dynamic_cast<OutPort *>(output_port);
  if (!op) {
    throw evaluation_error(
        "Cannot write to a non-port type: expected output-port");
  }
  // write the string representation of the object to the output port
  std::stringstream buf;
  auto repr = DisplayRepresentor(buf);
  msg->exec(repr);

  op->write(buf.str(), env);
  op->write("\n", env);

  return env.lookup("null");
}
SExp *primitive::close_output_port(std::list<SExp *> args, Env &env) {
  if (args.size() != 1) {
    throw evaluation_error(
        "Incorrect number of arguments in function close-output-port");
  }
  SExp *arg = args.front()->eval(env);
  OutPort *op = dynamic_cast<OutPort *>(arg);
  if (!op) {
    throw evaluation_error(
        "Invalid call of close-outport-port on non output port type");
  }
  op->close();
  return env.lookup("null");
}

SExp *primitive::modulo(std::list<SExp *> args, Env &env) {
  if (args.size() != 2) {
    throw evaluation_error("Invalid number of arguments in function %");
  }
  std::for_each(args.begin(), args.end(),
                [&env](SExp *&a) { a = a->eval(env); });

  Number *np;
  np = dynamic_cast<Number *>(args.front());
  double argument = np->val();
  if (!np) {
    throw evaluation_error("Encountered non-numeric arguments in function %");
  }
  args.pop_front();

  np = dynamic_cast<Number *>(args.front());
  double mod = np->val();
  if (!np) {
    throw evaluation_error("Encountered non-numeric arguments in function %");
  }
  double result = std::fmod(argument, mod);
  return env.manage(new Number(result));
}

SExp *primitive::not_stmt(std::list<SExp *> args, Env &env) {
  if (args.size() != 1) {
    throw evaluation_error("Incorrect number of arguments in function not");
  }
  auto x = args.front()->eval(env);
  bool result = !is_true(x);
  return env.manage(new Bool(result));
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

SExp *primitive::logical_and(std::list<SExp *> args, Env &env) {
  std::for_each(args.begin(), args.end(),
                [&env](SExp *&a) { a = a->eval(env); });
  bool result = true;
  for (auto it = args.begin(); it != args.end(); ++it) {
    if (!is_true(*it)) {
      result = false;
      break;
    }
  }
  return env.manage(new Bool(result));
}
SExp *primitive::logical_or(std::list<SExp *> args, Env &env) {
  std::for_each(args.begin(), args.end(),
                [&env](SExp *&a) { a = a->eval(env); });
  bool result = false;
  for (auto it = args.begin(); it != args.end(); ++it) {
    if (is_true(*it)) {
      result = true;
      break;
    }
  }
  return env.manage(new Bool(result));
}

static SExp *quote_var(SExp *var, Env &env) {
  return env.manage(new List(std::list<SExp *>{env.lookup("quote"), var}));
}

// (map f xs) where xs = (a b c d ...) --> ((f a) (f b) (f c) (f d) ...)
SExp *primitive::map(std::list<SExp *> args, Env &env) {
  if (args.size() != 2) {
    throw evaluation_error("Incorrect number of arguments in primitive map");
  }
  // evaluate arguments
  std::for_each(args.begin(), args.end(), [&](SExp *&a) { a = a->eval(env); });
  Function *func = dynamic_cast<Function *>(args.front());
  if (!func) {
    throw evaluation_error(
        "Illegal first argument in function map: expected function");
  }

  args.pop_front();
  List *list = dynamic_cast<List *>(args.front());
  if (!list) {
    throw evaluation_error(
        "Illegal second argument in function map: expected list");
  }

  std::list<SExp *> elements = list->elems;
  // apply the function func to every element in the list
  std::for_each(elements.begin(), elements.end(), [&func, &env](SExp *&a) {
    a = func->call(std::list<SExp *>{quote_var(a, env)}, env);
  });
  return env.manage(new List(elements));
}

SExp *primitive::filter(std::list<SExp *> args, Env &env) {
  if (args.size() != 2) {
    throw evaluation_error("Incorrect number of arguments in primitive filter");
  }
  // evaluate arguments
  std::for_each(args.begin(), args.end(), [&](SExp *&a) { a = a->eval(env); });
  Function *pred = dynamic_cast<Function *>(args.front());
  if (!pred) {
    throw evaluation_error(
        "Illegal first argument in function filter: expected function");
  }
  args.pop_front();
  List *list = dynamic_cast<List *>(args.front());
  if (!list) {
    throw evaluation_error(
        "Illegal second argument in function filter: expected list");
  }

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

  return env.manage(new List(elements));
}

// Implements a left fold over the list with the last element as the accumulator

//(fold (f acc x -> acc) acc (xs) )
SExp *primitive::fold(std::list<SExp *> args, Env &env) {
  if (args.size() != 3) {
    throw evaluation_error("Incorrect number of arguments in primitive fold");
  }
  // evaluate arguments
  std::for_each(args.begin(), args.end(), [&](SExp *&a) { a = a->eval(env); });

  Function *func = dynamic_cast<Function *>(args.front());
  if (!func) {
    throw evaluation_error(
        "Illegal first argument in function fold: expected function");
  }
  args.pop_front();

  SExp *init = args.front(); // the initial accumulator can be of any type
  args.pop_front();

  List *list = dynamic_cast<List *>(args.front());
  if (!list) {
    throw evaluation_error(
        "Illegal second argument in function fold: expected list");
  }

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

SExp *primitive::list(std::list<SExp *> args, Env &env) {
  // construct a list from elems: this is very simple!
  std::for_each(args.begin(), args.end(), [&](SExp *&a) { a = a->eval(env); });
  return env.manage(new List(args));
}
// convert a string to an s-expression
SExp *primitive::read(std::list<SExp *> args, Env &env) {
  // construct a list from elems: this is very simple!
  if (args.size() != 1) {
    throw evaluation_error(
        "Invalid number of arguments in function read: expected 1");
  }
  auto arg = args.front()->eval(env);
  String *sp = dynamic_cast<String *>(arg);
  if (!sp) {
    throw evaluation_error("Cannot read a non-string type");
  }
  std::string str = sp->val();

  // try to parse the string as an s-expression, returning its value
  // this function is problematic since there is basically no sensible
  // way to signal that this function has failed without adding exceptions
  // to the language
  auto buf = std::stringstream(str);
  auto parser = Parser(buf);
  return parser.read_sexp(env);
}
