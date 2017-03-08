
#include "env.h"
#include "sexp.h"
#include "primitives.h"
// for now, just implement numeric primitives

// TODO try to come up with a way to abstract the creation of these things? a
// lot of boilerplate here
// Maybe use a different helper function for special forms and functions?

// Function to abstract boilerplate in builtin function generation

// a function to abstract the creation of numeric primitives functions
SExp *GlobalEnv::mk_numeric_primitive(
    std::function<double(double acc, double x)> func, std::string funcname) {

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
        throw evaluation_error("Non numeric arguments "
                               "encountered in "
                               "function " +
                               funcname);
      }
      // now we know its a number, we are free to down-cast
      // the pointer to
      // access it's numeric field. We need to use get() to
      // access the raw
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

SExp* GlobalEnv::mk_builtin(std::function<SExp*(std::list<SExp*>, Env&)> fn, std::string funcname) {
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
  def("cons", mk_builtin(std::function<SExp* (std::list<SExp*>, Env& env)>(primitive_cons), "cons"));
  def("car", mk_builtin(std::function<SExp* (std::list<SExp*>, Env& env)>(primitive_car), "car"));
  def("quote", mk_builtin(std::function<SExp* (std::list<SExp*>, Env& env)>(primitive_quote), "quote"));
  def("define", mk_builtin(std::function<SExp* (std::list<SExp*>, Env& env)>(primitive_define), "define"));
  def("lambda", mk_builtin(std::function<SExp* (std::list<SExp*>, Env& env)>(primitive_lambda), "lambda"));
  def("cdr", mk_builtin(std::function<SExp* (std::list<SExp*>, Env& env)>(primitive_cdr), "cdr"));
  def("if", mk_builtin(std::function<SExp* (std::list<SExp*>, Env& env)>(primitive_if), "if"));
  def("null?", mk_builtin(std::function<SExp* (std::list<SExp*>, Env& env)>(primitive_isnull), "null?"));
  def("exit", mk_builtin(std::function<SExp* (std::list<SExp*>, Env& env)>(primitive_exit), "exit"));
  def("=", mk_builtin(std::function<SExp* (std::list<SExp*>, Env& env)>(primitive_numeric_eq), "="));
  def("eq?", mk_builtin(std::function<SExp* (std::list<SExp*>, Env& env)>(primitive_eq), "eq?"));
  return;
}

GlobalEnv::~GlobalEnv() {}

Env GlobalEnv::capture_scope() {
  // create a new Env, closing over the current scope.
  return Env(*this);
}

Env::Env(GlobalEnv &g) : global(&g) { scope = g.scope; }

// defineing is only legal in the global scope
void Env::def(std::string id, SExp *value) {
  // bind to the global scope
  scope[id] = value;
  // auto repr = Representor(std::cout);
  // std::cout << "Defining value " << id << " as ";
  // value->exec(repr);
  // std::cout << std::endl;
  return;
}

SExp *Env::allocate(SExp *new_obj) {
  // keep all control of allocation in the global scope
  return global->allocate(new_obj);
}
