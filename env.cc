
#include "env.h"
// for now, just implement numeric primitives

// TODO change all functions to be non-destructive, taking a reference to their
// argument and copying when neccesary instead.

// a function to abstract the creation of numeric primitives functions
SExp *
Env::mk_numeric_primitive(std::function<double(double acc, double x)> func,
                          std::string funcname) {

  // generate a function object to apply the primitive to a lisp list
  // make this a shared pointer? this is to avoid copying the actual procedure
  // every time when we
  // do a primitive lookup: instead allocate on the head and pass around a
  // reference
  // Is this even a big deal?
  auto const fn = [func, funcname](std::list<SExp *> args, Env &env) -> SExp * {
    // check the list is not empty
    if (args.empty()) {
      throw evaluation_error("Incorrect number of arguments in function " +
                             funcname);
    }
    for (auto it = args.begin(); it != args.end(); ++it) {
      (*it) = (*it)->eval(env);
    }
    double acc;
    for (auto it = args.begin(); it != args.end(); ++it) {
      if ((*it)->type() != LispType::Number) {
        throw evaluation_error(
            "Non numeric arguments encountered in function " + funcname);
      }
      // now we know its a number, we are free to down-cast the pointer to
      // access it's numeric field. We need to use get() to access the raw
      // pointer managed by the unique pointer wrapper
      double num = (static_cast<Number *>(*it))->val();

      if (it == args.begin()) {
        acc = num;
      } else {
        acc = func(acc, num);
      }
    }
    SExp *result = env.allocate(new Number(acc));
    return result;
  };
  return heap.allocate(new PrimitiveFunction(fn, funcname));
}

// called to create a blank environment: bind the language builtins
Env::Env() : enclosing_scope(nullptr) { bind_primitives(); }

// this creates a new closure, pointing to the one that created it.
Env::Env(Env *env) {
  enclosing_scope = env;
  return;
}

SExp *Env::lookup(std::string id) {
  auto x = scope.find(id);
  if ((x == scope.end())) {
    if (enclosing_scope != nullptr) {
      // if theres a reference to a parent scope, try looking stuff up in that
      // if this lookup failed
      return enclosing_scope->lookup(id);
    } else {
      // undefined
      return nullptr;
    }
  } else {
    return x->second;
  }
} // void Env::def(std::string id, std::function<SExp *(std::list<sexp_ptr>)>
// func) {
//  scope[id] = func;
//}
//
void Env::bind_primitives() {
  scope["+"] = mk_numeric_primitive(
      [](double acc, double x) -> double { return acc + x; }, "+");
  scope["-"] = mk_numeric_primitive(
      [](double acc, double x) -> double { return acc - x; }, "-");
  scope["*"] = mk_numeric_primitive(
      [](double acc, double x) -> double { return acc * x; }, "*");
  scope["/"] = mk_numeric_primitive(
      [](double acc, double x) -> double { return acc / x; }, "/");
  scope["cons"] = mk_cons();
  scope["car"] = mk_car();
  scope["quote"] = mk_quote();
  scope["define"] = mk_define();
  return;
}

Env::~Env() {
  heap = Heap();
  // not sure how much I like this: should we really store raw pointers in the
  // env?
  // Also not sure how much we should rely on copy semantics: atm I'm fairly
  // sure
  // we are just returning a pointer to some static memory. Clearly this won't
  // work
  // on things like lists, or indeed anything where we will want to take over
  // ownership of an object.
  // for (auto it = scope.begin(); it != scope.end(); ++it) {
  //  delete it->second;
  //}
}

void Env::def(std::string id, SExp *value) {
  scope[id] = value;
  auto repr = Representor(std::cout);
  std::cout << "Defining value " << id << " as ";
  value->exec(repr);
  std::cout << std::endl;
  return;
}
// TODO can maybe change all these to return sexp_ptr, and hand of the change of
// control to maybe_reset? maybe safer
SExp *Env::mk_cons() {
  auto cons = [](std::list<SExp *> args, Env &env) -> SExp * {
    if (args.size() != 2) {
      throw evaluation_error("Incorrect number of arguments in primitive cons");
    }
    for (auto it = args.begin(); it != args.end(); ++it) {
      (*it) = (*it)->eval(env);
    }
    SExp *car = args.front();
    args.pop_front();
    SExp *cdr = args.front();
    // technically this is different from classical lisp as we don't have a
    // dotted-list type (which I don't see as particularly useful)
    if (cdr->type() != LispType::List) {
      throw evaluation_error(
          "Cannot cons onto a non-list type [expected: (cons T list)]");
    }
    // push the car argument onto the list of the second, then return the second
    // argument.
    // Copy the list: this only copies the pointers, so it should be relatively
    // cheap
    auto list_cdr = static_cast<List *>(cdr);
    SExp *cons_list = env.allocate(new List(*list_cdr));
    static_cast<List *>(cons_list)->elems.push_front(car);
    return cons_list;
  };
  return heap.allocate(new PrimitiveFunction(cons, "cons"));
}

SExp *Env::mk_car() {
  auto car = [](std::list<SExp *> args, Env &env) {
    if (args.size() != 1) {
      throw evaluation_error("Incorrect number of arguments in primitive car");
    }
    SExp *ls = args.front();
    args.pop_front();
    ls = ls->eval(env);
    if (ls->type() != LispType::List) {
      throw evaluation_error("Cannot take the car of a non-list");
    }
    auto car = static_cast<List *>(ls)->elems.front();
    return car;
  };
  SExp *primitive_car = heap.allocate(new PrimitiveFunction(car, "car"));
  return primitive_car;
}

SExp *Env::mk_quote() {
  auto quote = [](std::list<SExp *> args, Env &env) -> SExp * {
    if (args.size() != 1) {
      throw evaluation_error(
          "Incorrect number of arguments in primitive quote");
    }
    SExp *atom = args.front();
    return atom;
  };
  SExp *primitive_quote = heap.allocate(new PrimitiveFunction(quote, "quote"));
  return primitive_quote;
}

SExp *Env::mk_define() {
  auto define = [](std::list<SExp *> args, Env &env) -> SExp * {
    if (args.size() != 2) {
      throw evaluation_error(
          "Incorrect number of arguments in primitive define: expected two");
    }
    auto atom = args.front();
    args.pop_front();
    if (atom->type() != LispType::Atom) {
      throw evaluation_error(
          "Expected atomic symbol as first argument to define");
    }
    std::string id = static_cast<Atom *>(atom)->get_identifier();
    auto value = args.front();
    value = value->eval(env);
    env.def(id, value);
    return env.allocate(new List);
  };
  SExp *primitive_define =
      heap.allocate(new PrimitiveFunction(define, "define"));
  return primitive_define;
}