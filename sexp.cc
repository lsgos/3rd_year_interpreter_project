#include "sexp.h"
#include "env.h"
#include "lisp_exceptions.h"
#include <fstream>
#include <list>
#include <memory>
#include <sstream>

// This is used to implement the 'truthy' behaviour of lisp: all expressions can
// be substituted for booleans, and everything that isn't #f (false) is true
bool is_true(SExp *exp) {
  return !(exp->type() == LispType::Bool &&
           static_cast<Bool *>(exp)->val() == false);
}

// helper functions allowing casts to the function abstract class
bool is_function(LispType type) {
  return (type == LispType::PrimitiveFunction ||
          type == LispType::LambdaFunction);
}

SExp *Atom::eval(Env &env) {
  auto value = env.lookup(id);
  if (value) {
    return value;
  } else {
    throw evaluation_error("Encountered undefined atom " + id);
  }
}

SExp *List::eval(Env &env) {
  if (elems.empty()) {
    throw evaluation_error("Cannot evaluate the empty list");
  }
  SExp *head = elems.front()->eval(env);

  auto args = elems;
  args.pop_front(); // first argument is head

  if (!is_function(head->type())) {
    throw evaluation_error("Expected function as first argument");
  }
  auto func = static_cast<Function *>(head);
  SExp *result = func->call(args, env);
  return result;
}

SExp *LambdaFunction::call(std::list<SExp *> args, Env &env) {
  // check the argument list matches the params of the function
  if (args.size() != params.size()) {
    std::stringstream msg;
    auto repr = Representor(msg);
    msg << "Found mismatched argument list in function ";
    this->exec(repr); // print function name to msg string
    msg << ", Expected " << params.size() << ", found " << args.size();
    throw evaluation_error(msg.str());
  }
  // make a copy of the captured environment to evaluate the function call
  auto f_env = closure;

  // evaluate the arguments and bind them to the function parameter names
  // in the function environment

  auto arg = args.begin();
  for (auto par = params.begin(); par != params.end(); ++par, ++arg) {
    (*arg) = (*arg)->eval(env);
    f_env.def(*par, *arg);
  }
  SExp *result;
  // evaluate the body of the function, returning the result of the last
  // expression
  for (auto it = body.begin(); it != body.end(); ++it) {
    result = (*it)->eval(f_env);
  }
  return result;
}

InPort::InPort(std::string name) : stdin(false), name(name) {
  file.open(name);
  if (!file.is_open()) {
    throw io_error("Cannot open file " + name);
  }
}

void InPort::close() {
  if (file.is_open()) {
    file.close();
  }
}

InPort::~InPort() { this->close(); }

// read the entire file contents into a string
SExp *InPort::read(Env &env) {
  // check the file state is ok before reading
  if (!stdin) {
    if (!file.is_open()) {
      throw io_error("Invalid attempt to read from closed file");
    } else if (!file.good()) {
      throw io_error("Found output file in invalid state");
    }
  }
  std::istream &in = stdin ? std::cin : file;
  // this is exploiting the fact that the std::istreambuf_iterator's default
  // constructor leaves it in
  // the eof state, thus this will read from the file until eof is reached,
  // constructing str as it does
  auto str = std::string((std::istreambuf_iterator<char>(in)),
                         std::istreambuf_iterator<char>());
  return env.allocate(new String(str));
}

SExp *InPort::read_ln(Env &env) {
  if (!stdin) {
    if (!file.is_open()) {
      throw io_error("Invalid attempt to read from closed file");
    } else if (!file.good()) {
      throw io_error("Found output file in invalid state");
    }
  }
  std::istream &in = stdin ? std::cin : file;
  std::string str;
  std::getline(in, str);
  return env.allocate(new String(str));
}

OutPort::OutPort(std::string name) : stdoutput(false), name(name) {
  // we want the file to be open as long as this object exists, so the program
  // maintains control over the resource
  file.open(name);
  if (!file.is_open()) {
    throw io_error("Cannot open file" + name);
  }
}

OutPort::~OutPort() { this->close(); }

void OutPort::close() {
  if (file.is_open()) {
    file.close();
  }
}
// write to the file object handled by outstream
SExp *OutPort::write(std::string str, Env &env) {
  if (stdoutput) {
    std::cout << str;
  } else {
    if (!file.good())
      throw io_error("Invalid write to file " + name);
    if (!file.is_open())
      throw io_error("Invalid write to closed file " + name);
    file << str;
  }
  return env.lookup("null");
}

void Representor::visit(Number &number) { stream << number.val(); }
void Representor::visit(String &string) {
  stream << "\"";
  stream << string.val();
  stream << "\"";
}
void Representor::visit(Bool &boolean) {
  bool v = boolean.val();
  if (v) {
    stream << "#t";
  } else {
    stream << "#f";
  }
}
void Representor::visit(Atom &atom) { stream << atom.get_identifier(); }
void Representor::visit(List &list) {
  stream << "(";
  if (!list.elems.empty()) {
    list.elems.front()->exec(*this);
    // void representor on all elements in the list.
    for (auto x = ++list.elems.begin(); x != list.elems.end(); x++) {
      stream << " ";
      (*x)->exec(*this);
    }
  }
  stream << ")";
}

void Representor::visit(PrimitiveFunction &fn) {
  stream << "<primitive " << fn.get_name() << ">";
}

void Representor::visit(LambdaFunction &lambda) {
  // e.g <lambda x y>
  stream << "<lambda ";
  if (!lambda.params.empty()) {
    stream << lambda.params.front();
    for (auto it = ++lambda.params.begin(); it != lambda.params.end(); it++) {
      stream << " ";
      stream << *it;
    }
  }
  stream << ">";
}

void Representor::visit(InPort &in) {
  stream << "<InPort " << in.get_name() << ">";
}

void Representor::visit(OutPort &out) {
  stream << "< output-port " << out.get_name() << ">";
}

// specialised version for printing the literal content of strings
void DisplayRepresentor::visit(String &string) { stream << string.val(); }

void DisplayRepresentor::visit(List &list) {
  // should display elements of a list as a normal list, not a printed string
  auto repr = Representor(stream);
  list.exec(repr);
}

std::ostream &operator<<(std::ostream &os, SExp &sexp) {
  auto repr = Representor(os);
  sexp.exec(repr);
  return os;
}