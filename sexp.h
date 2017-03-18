#ifndef SEXP_H
#define SEXP_H

#include "env.h"
#include <fstream>
#include <functional>
#include <iostream>
#include <list>
#include <memory>
#include <string>
#include <unordered_map>
// This header file is the core of the language, defining the allowed builtin
// types
// All valid LISP expressions are s-expressions,
// or symbolic expressions, either primitive datatypes or lists of them.
// Lisp data is represented by an abstract class which the datatypes
// inherit from.
// forward declaration of the env class to avoid circular dependency:
// see env.h for details
// class Env;
class SExp;

class Number;
class String;
class Bool;
class Atom;
class List;
class PrimitiveFunction;
class LambdaFunction;
class InPort;
class OutPort;

enum class LispType {
  Number,
  String,
  Bool,
  Atom,
  List,
  PrimitiveFunction,
  LambdaFunction,
  InPort,
  OutPort,
};

bool is_function(LispType type);
bool is_true(SExp *);
// Visitor function. This allows classes that recurse through sexp
// trees changing the internal state of the visitor class. At
// present only useful for the representation function.
class SExpVisitor {
public:
  virtual void visit(Number &number) = 0;
  virtual void visit(String &string) = 0;
  virtual void visit(Atom &atom) = 0;
  virtual void visit(Bool &boolean) = 0;
  virtual void visit(List &list) = 0;
  virtual void visit(PrimitiveFunction &fn) = 0;
  virtual void visit(LambdaFunction &lambda) = 0;
  virtual void visit(InPort &in) = 0;
  virtual void visit(OutPort &out) = 0;
};

// abstract class representing list data
class SExp {
public:
  // a function allowing visitor classes
  virtual void exec(SExpVisitor &visitor) = 0;
  // eval returns a pointer to an SExp containing the
  // value of the lisp expression. May be the same as the
  // current pointer (primitive datatypes are their own values),
  // the value defined in the symbol table (for atoms) or
  // a new value constructed by function evaluation (for lists).
  // Garbage collection is handled by the Env class
  virtual SExp *eval(Env &env) = 0;
  // returns an enum value giving which subclass of SExp an instantiation
  // of
  // this interface actually is. This is needed when one expects a
  // specific
  // type
  // (e.g numbers for numeric primitives, functions for function
  // application)
  // and one has to downcast to the type from the generic in order to
  // extract
  // its data. Checking with this first preserves safety
  virtual LispType type() const = 0;
  virtual ~SExp() {}
};

// template class for 'primitive' types, like numbers and booleans. Primitive
// types
// all essentially just box a c++ value, and they all eval to themselves, so
// most
// of the boilerplate of creating these classes can be abstracted to a template.
// The intantiated templates have to implement the exec visitor function though
template <typename T, LispType kind> class PrimitiveType : public SExp {
  const T value;

public:
  PrimitiveType<T, kind>(T value) : value(value) {}
  ~PrimitiveType() {}
  T val() { return value; }
  LispType type() const override { return kind; }
  virtual SExp *eval(Env &env) override { return this; }
};

class Number : public PrimitiveType<double, LispType::Number> {
public:
  Number(double x) : PrimitiveType<double, LispType::Number>(x) {}
  void exec(SExpVisitor &visitor) override { visitor.visit(*this); }
};

class String : public PrimitiveType<std::string, LispType::String> {
public:
  String(std::string str) : PrimitiveType<std::string, LispType::String>(str) {}
  void exec(SExpVisitor &visitor) override { visitor.visit(*this); }
};

class Bool : public PrimitiveType<bool, LispType::Bool> {
public:
  Bool(bool x) : PrimitiveType<bool, LispType::Bool>(x) {}
  void exec(SExpVisitor &visitor) override { visitor.visit(*this); }
};

// Atoms represent symbols. While the actual data is a string, an atom
// is semantically totally different: it is evaluated by replacing it
// with it's defined value in the symbol table in env.
class Atom : public SExp {
public:
  Atom(std::string id) : id(id) {}
  ~Atom() {}
  void exec(SExpVisitor &visitor) override { visitor.visit(*this); }
  std::string get_identifier() { return id; }
  virtual SExp *eval(Env &env) override;
  LispType type() const override { return LispType::Atom; }

private:
  const std::string id;
};

// Linked lists. Lists are eval'ed as function calls, of the form
//(f arg1 arg2 ...)
class List : public SExp {
public:
  List(std::list<SExp *> list) : elems(list) {}
  void exec(SExpVisitor &visitor) override { visitor.visit(*this); }
  const std::list<SExp *> elems;
  virtual SExp *eval(Env &env) override;
  LispType type() const override { return LispType::List; }
  ~List() override {}

  List() {}
};

// interface to represent lisp function objects.
class Function : public SExp {
public:
  virtual SExp *call(std::list<SExp *>, Env &) = 0;
  virtual ~Function() {}
};

class PrimitiveFunction : public Function {
private:
  const std::function<SExp *(std::list<SExp *> &, Env &)> fn;
  const std::string name;

public:
  PrimitiveFunction(const std::function<SExp *(std::list<SExp *> &, Env &)> fn,
                    std::string name)
      : fn(fn), name(name) {}

  std::string get_name() { return name; }
  virtual SExp *eval(Env &env) override { return this; }
  void exec(SExpVisitor &visitor) override { visitor.visit(*this); }

  virtual SExp *call(std::list<SExp *> args, Env &env) override {
    return fn(args, env);
  }
  LispType type() const override { return LispType::PrimitiveFunction; }

  ~PrimitiveFunction() override {}
};

// user defined function closures.
class LambdaFunction : public Function {
private:
  const std::list<std::string> params;
  std::list<SExp *> body;
  const Env closure;

public:
  // constructor: caputures a scope, a list of parameter names and a body,
  // a
  // sequence of SExps to execute when the function is called.
  LambdaFunction(Env env, std::list<std::string> params, std::list<SExp *> body)
      : closure(env), params(params), body(body) {}
  virtual SExp *call(std::list<SExp *> args, Env &env) override;
  LispType type() const override { return LispType::LambdaFunction; }
  void exec(SExpVisitor &visitor) override { visitor.visit(*this); }
  SExp *eval(Env &env) override { return this; }
  ~LambdaFunction() override {}
  friend class Heap; // needs to access the env and body of lambdas for
                     // garbage
                     // collection
  friend class Representor;
};

// Handles to input and output streams //TODO finish implementing this

class InPort : public SExp {
private:
  std::string name;
  bool stdin;
  std::ifstream file;

public:
  InPort() : stdin(true) {}
  InPort(std::string name);
  LispType type() const override { return LispType::InPort; }
  SExp *read(Env &env);
  SExp *read_char(Env &env);
  SExp *read_ln(Env &env);
  SExp *eval(Env &env) override { return this; }
  void exec(SExpVisitor &visitor) override { visitor.visit(*this); }
  std::string get_name() { return name; }
  void close();
  ~InPort();
};

class OutPort : public SExp {
private:
  std::ofstream file;
  bool stdoutput;
  std::string name;

public:
  OutPort() : stdoutput(true), name("stdout") {}
  OutPort(std::string name);
  LispType type() const override { return LispType::OutPort; }
  SExp *write(std::string, Env &env);
  SExp *eval(Env &env) override { return this; }
  void exec(SExpVisitor &visitor) override { visitor.visit(*this); }
  std::string get_name() { return name; }
  void close();
  ~OutPort();
};

// visitor class, writes a representation of an s-expression to a stream
class Representor : public SExpVisitor {
protected:
  std::ostream &stream;

public:
  Representor(std::ostream &stream) : stream(stream) {}
  void visit(String &string);
  void visit(Number &number);
  void visit(Bool &boolean);
  void visit(Atom &atom);
  void visit(List &list);
  void visit(PrimitiveFunction &fn);
  void visit(LambdaFunction &lambda);
  void visit(InPort &in);
  void visit(OutPort &out);
};

// implement the stream insertion operator for Sexps using the representor class
std::ostream &operator<<(std::ostream &os, SExp &sexp);

// There should be a different printing behaviour for strings only when they are
// printed
// as formatted output: they should be displayed literally rather than enclosed
// in quotes.
// Everything else can be inherited from the basic representor.
class DisplayRepresentor : public Representor {
public:
  DisplayRepresentor(std::ostream &os) : Representor(os) {}
  void visit(String &string);
  void visit(List &list);
};
#endif