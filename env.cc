
#include "env.h"
#include "primitives.h"
#include "sexp.h"

// a function to abstract the creation of numeric primitive functions
SExp *GlobalEnv::mk_numeric_primitive(
    std::function<double(double acc, double x)> func, std::string funcname) {

  auto const fn = [func, funcname](std::list<SExp *> args, Env &env) -> SExp * {
    // check the list is not empty
    if (args.empty()) {
      throw evaluation_error("Incorrect number of arguments in function " +
                             funcname);
    }
    std::for_each(args.begin(), args.end(),
                  [&](SExp *&a) { a = a->eval(env); });
    double acc;
    for (auto it = args.begin(); it != args.end(); ++it) {
    
      Number* nptr = dynamic_cast<Number *>(*it);
         
      if (!nptr) {
        throw evaluation_error("Non numeric arguments "
                               "encountered in "
                               "function " +
                               funcname);
      }
      double num = nptr->val();

      if (it == args.begin()) {
        acc = num;
      } else {
        acc = func(acc, num);
      }
    }
    SExp *result = env.manage(new Number(acc));
    return result;
  };
  return heap.manage(new PrimitiveFunction(fn, funcname));
}

// takes a function and converts it into a PrimitiveFunction object containing
// it
SExp *GlobalEnv::mk_builtin(std::function<SExp *(std::list<SExp *>, Env &)> fn,
                            std::string funcname) {
  return heap.manage(new PrimitiveFunction(fn, funcname));
}

// called to create a blank environment: bind the language builtins
GlobalEnv::GlobalEnv() { bind_primitives(); }

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
  using namespace primitive;
  // constant null
  def("null", heap.manage(new List()));

  // primitive functions
  def("+", mk_numeric_primitive(
               [](double acc, double x) -> double { return acc + x; }, "+"));
  def("-", mk_numeric_primitive(
               [](double acc, double x) -> double { return acc - x; }, "-"));
  def("*", mk_numeric_primitive(
               [](double acc, double x) -> double { return acc * x; }, "*"));
  def("/", mk_numeric_primitive(
               [](double acc, double x) -> double { return acc / x; }, "/"));
  def("cons", mk_builtin(cons, "cons"));
  def("car", mk_builtin(car, "car"));
  def("quote", mk_builtin(quote, "quote"));
  def("define", mk_builtin(define, "define"));
  def("lambda", mk_builtin(lambda, "lambda"));
  def("cdr", mk_builtin(cdr, "cdr"));
  def("if", mk_builtin(if_stmt, "if"));
  def("null?", mk_builtin(isnull, "null?"));
  def("exit", mk_builtin(exit_stmt, "exit"));
  def("=", mk_builtin(numeric_eq, "="));
  def("eq?", mk_builtin(eq, "eq?"));
  def("eval", mk_builtin(eval, "eval"));
  def("number?", mk_builtin(is_number, "number?"));
  def("open-output-port", mk_builtin(open_output_port, "open-output-port"));
  def("display", mk_builtin(display, "display"));
  def("displayln", mk_builtin(displayln, "displayln"));
  def("close-output-port", mk_builtin(close_output_port, "close-output-port"));
  // bind standard output and input to lisp input and output objects
  def("std-output-port", heap.manage(new OutPort()));
  def("%", mk_builtin(modulo, "%"));
  def("not", mk_builtin(not_stmt, "not"));
  def("map", mk_builtin(map, "map"));
  def("filter", mk_builtin(filter, "filter"));
  def("fold", mk_builtin(fold, "fold"));
  def("list", mk_builtin(list, "list"));
  def("and", mk_builtin(logical_and, "and"));
  def("or", mk_builtin(logical_or, "or"));
  def("open-input-port", mk_builtin(open_input_port, "open-input-port"));
  def("close-input-port", mk_builtin(close_input_port, "close-input-port"));
  def("port->string", mk_builtin(port_to_string, "port->string"));
  def("read", mk_builtin(read, "read"));
  return;
}

// bind command line arguments when a script is called to a list
void GlobalEnv::bind_argv(int argc, char *argv[]) {
  std::list<SExp *> arglist;
  for (int i = 1; i < argc; i++) {
    arglist.push_back(heap.manage(new String(argv[i])));
  }
  def("ARGV", heap.manage(new List(arglist)));
}

GlobalEnv::~GlobalEnv() {}

Env GlobalEnv::capture_scope() {
  // create a new Env, closing over the current scope.
  return Env(*this);
}

// note that this stores a reference back to the scope
// that created it: hence why global cannot move
Env::Env(GlobalEnv &g) : global(&g) { scope = g.scope; }

void Env::def(std::string id, SExp *value) {
  // bind to the global scope
  scope[id] = value;
  // auto repr = Representor(std::cout);
  // std::cout << "Defining value " << id << " as ";
  // value->exec(repr);
  // std::cout << std::endl;
  return;
}

SExp *Env::manage(SExp *new_obj) {
  // keep all control of allocation in the global scope
  return global->manage(new_obj);
}
