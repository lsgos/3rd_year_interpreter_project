
#include "env.h"
#include "sexp.h"
// for now, just implement numeric primitives

// TODO change all functions to be non-destructive, taking a reference to their
// argument and copying when neccesary instead.

// a function to abstract the creation of numeric primitives functions
SExp *GlobalEnv::mk_numeric_primitive(
    std::function<double(double acc, double x)> func, std::string funcname) {

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
GlobalEnv::GlobalEnv() { bind_primitives(); }

// this creates a new closure, pointing to the one that created it.
SExp *Env::lookup(std::string id) {

  auto x = scope.find(id);
  if (x != scope.end())
    return x->second;

  if (global != nullptr) {
    // if there is a link-back to a global scope
    return global->lookup(id);
  }
  return nullptr;
}

void GlobalEnv::bind_primitives() {
  def("+", mk_numeric_primitive(
               [](double acc, double x) -> double { return acc + x; }, "+"));
  def("-", mk_numeric_primitive(
               [](double acc, double x) -> double { return acc - x; }, "-"));
  def("*", mk_numeric_primitive(
               [](double acc, double x) -> double { return acc * x; }, "*"));
  def("/", mk_numeric_primitive(
               [](double acc, double x) -> double { return acc / x; }, "/"));
  def("cons", mk_cons());
  def("car", mk_car());
  def("quote", mk_quote());
  def("define", mk_define());
  def("lambda", mk_lambda());
  return;
}

GlobalEnv::~GlobalEnv() {}

Env GlobalEnv::capture_scope() {
  // create a new Env, closing over the current scope.
  return Env(*this);
}

Env::Env(GlobalEnv &g) : global(&g) {
  scope = g.scope;
  // is the pointer back to the global scope even neccesary? we just shadowed
  // all the variables by cloning the table in
  // any case, which is neccesary for proper function closures...
}

// defineing is only legal in the global scope
void Env::def(std::string id, SExp *value) {
  // bind to the global scope
  scope[id] = value;
  //auto repr = Representor(std::cout);
  //std::cout << "Defining value " << id << " as ";
  //value->exec(repr);
  //std::cout << std::endl;
  return;
}

SExp *Env::allocate(SExp *new_obj) {
  // keep all control of allocation in the global scope
  return global->allocate(new_obj);
}
SExp *GlobalEnv::mk_cons() {
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
    std::list<SExp *> cons_list = list_cdr->elems;
    cons_list.push_back(car);
    return env.allocate(new List(cons_list));
  };
  return heap.allocate(new PrimitiveFunction(cons, "cons"));
}

SExp *GlobalEnv::mk_car() {
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

SExp *GlobalEnv::mk_quote() {
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

SExp *GlobalEnv::mk_define() {
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

SExp *GlobalEnv::mk_lambda() {
  auto lambda= [](std::list<SExp *> args, Env &env) -> SExp * {
    // lambda is a special form: evaluate the first argument, but none of the
    // others.
    if (args.size() < 2) {
      throw evaluation_error("Too few arguments in call to lambda");
    }
    // first argument to lambda must be either a list of atoms or a single atom
    SExp* first = args.front();
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
          }
        else {
          throw evaluation_error("Error in arguments to lambda: excepted "
                                 "identifier or list of identifiers");
        }
      }
    } else {
      // not valid:
      throw evaluation_error("Error in first argument to lambda: expected "
                             "identifier or list of identifiers");
    }
    //capture scope
    Env closure = env.capture_scope();
    return env.allocate(new LambdaFunction(closure, param_list, args));
  };
  SExp * primitive_lambda = heap.allocate(new PrimitiveFunction(lambda, "lambda"));
  return primitive_lambda;
}

SExp *Heap::allocate(SExp *new_object) {
  // objects.push_back(Cell(new_object));
  std::pair<SExp *, bool> entry(new_object, false);
  //std::cout << "Storing " << new_object << std::endl;
  objects.insert(entry);
  return new_object;
}

// The heap class is responsible for manageing the memory usage of
// the program, so it needs to clean up the pointers.
Heap::~Heap() {
  for (auto it = objects.begin(); it != objects.end(); ++it) {
    //std::cout << "Freeing " << it->first << std::endl;
    delete it->first;
  }
}

// setup the heap for mark and sweep: set all the usage marks to zero
void Heap::reset_marks() {
  for (auto it = objects.begin(); it != objects.end(); ++it) {
    it->second = false;
  }
}
// sweep memory, cleaning up anything marked for deletion.
void Heap::sweep() {
  for (auto it = objects.begin(), next = objects.begin(); it != objects.end();
       it = next) {
    next = it;
    ++next;

    if (!(it->second)) {
      SExp *ptr = it->first; // cleanup memory managed by this key
      objects.erase(it);
      //std::cout << "Freeing " << ptr << std::endl;
      delete ptr; // remove from the heap
    }
  }
}

void Heap::mark(SExp *addr) {
  auto entry = objects.find(addr);
  if (entry == objects.end()) {
    throw implementation_error(
        "This shouldn't happen: encountered unmanaged address");
  }
  // mark memory as used
  entry->second = true;
  //std::cout << "Marked " << entry->first << " as in use" << std::endl;
  auto expr_type = entry->first->type();
  // Lists and functions can contain references to other objects: we need to
  // recursively trace out their tree.
  // can we implement this as a visitor? might be slightly neater. Or is it
  // unnessarily complicated?
  if (expr_type == LispType::List) {
    auto list = static_cast<List *>(addr);
    for (auto it = list->elems.begin(); it != list->elems.end(); ++it) {
      // mark all sub elements as well
      mark(*it);
    }
  }
  if (expr_type == LispType::LambdaFunction) {
    auto lambda = static_cast<LambdaFunction *>(addr);
    // mark the expressions in the function body
    for (auto obj = lambda->body.begin(); obj != lambda->body.end(); ++obj) {
      mark(*obj);
    }
    // iterate through the closure's bindings, marking the scope captured from
    // it's creation
    for (auto obj = lambda->closure.scope.begin();
         obj != lambda->closure.scope.end(); ++obj) {
      mark(obj->second);
    }
  }
  return;
}

// naive mark-and-sweep: take the environment bindings, and walk through the
// objects described by their bindings, marking everything reachable from the
// root nodes as in use. Then iterate through the object map freeing everything
// that wasn't reachable; these things have no bindings to the rest of the
// program
// and are thus garbage.
void Heap::collect_garbage(Env &env) {
  reset_marks();
  for (auto entry = env.scope.begin(); entry != env.scope.end(); ++entry) {
    mark(entry->second);
  }
  sweep();
}