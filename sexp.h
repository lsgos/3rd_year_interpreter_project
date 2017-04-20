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

// Abstract class for language objects
class SExp {
public:
  // a function allowing visitor classes to dispatch
  // methods based on the underlying type of the expression
  virtual void exec(SExpVisitor &visitor) = 0;
  
  // eval returns a pointer to an SExp containing the
  // result of evaluating the lisp expression.
  virtual SExp *eval(Env &env) = 0;
  virtual ~SExp() {}
};

// template class for 'primitive' types, like numbers and booleans. Primitive
// types all essentially just box a c++ value, and they all eval to themselves,
// so most of the boilerplate of creating these classes can be abstracted to a
// template
template <typename T> class PrimitiveType : public SExp {
  const T value;

public:
  PrimitiveType<T>(T value) : value(value) {}
  ~PrimitiveType() {}
  T val() { return value; }
  virtual SExp *eval(Env &env) override { return this; }
};

class Number : public PrimitiveType<double> {
public:
  Number(double x) : PrimitiveType<double>(x) {}
  void exec(SExpVisitor &visitor) override { visitor.visit(*this); }
};

class String : public PrimitiveType<std::string> {
public:
  String(std::string str) : PrimitiveType<std::string>(str) {}
  void exec(SExpVisitor &visitor) override { visitor.visit(*this); }
};

class Bool : public PrimitiveType<bool> {
public:
  Bool(bool x) : PrimitiveType<bool>(x) {}
  void exec(SExpVisitor &visitor) override { visitor.visit(*this); }
};

// Atoms represent symbols. While the actual data is a string, an atom
// is semantically totally different: it is evaluated by replacing it
// with the value corresponding to it in the symbol table (see env.h)

class Atom : public SExp {
public:
  Atom(std::string id) : id(id) {}
  ~Atom() {}
  void exec(SExpVisitor &visitor) override { visitor.visit(*this); }
  std::string get_identifier() { return id; }
  virtual SExp *eval(Env &env) override;

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
  ~List() override {}
  List() {}
};

// interface to represent lisp function objects.
class Function : public SExp {
public:
  virtual SExp *call(std::list<SExp *>, Env &) = 0;
  virtual ~Function() {}
};

//Builtin functions
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
  ~PrimitiveFunction() override {}
};

// user defined functions
class LambdaFunction : public Function {
private:
  const std::list<std::string> params;
  std::list<SExp *> body;
  const Env closure;

public:
  LambdaFunction(Env env, std::list<std::string> params, std::list<SExp *> body)
      : closure(env), params(params), body(body) {}
  virtual SExp *call(std::list<SExp *> args, Env &env) override;
  void exec(SExpVisitor &visitor) override { visitor.visit(*this); }
  SExp *eval(Env &env) override { return this; }
  ~LambdaFunction() override {}
  friend class Heap; // needs to access the env and body of lambdas for
                     // garbage collection

  friend class Representor;
};

// Handles to input and output streams

class InPort : public SExp {
private:
  std::string name;
  bool stdin;
  std::ifstream file;

public:
  InPort() : stdin(true) {}
  InPort(std::string name);
  SExp *read(Env &env);
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
  SExp *write(std::string, Env &env);
  SExp *eval(Env &env) override { return this; }
  void exec(SExpVisitor &visitor) override { visitor.visit(*this); }
  std::string get_name() { return name; }
  void close();
  ~OutPort();
};

// The representor class is used to write the s-expressions to a stream. It is
// written using the 'visitor pattern', a way of decoupling operations on
// classes from the object structure. By calling sexp->exec(*this), a visitor
// despatches it's own visit method on the actual underlying sexp object. It
// looks a little bit complicated but it all works out.

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

// implement the stream insertion operator for sexps using the representor class
std::ostream &operator<<(std::ostream &os, SExp &sexp);

// A slightly different version of the representor, which is used when printing
// using the display function. The only difference is that strings are printed
// with their actual values (so "hello" is printed as hello instead of "hello").
// Notice that the use of the visitor pattern means the sexp heirarchy doesn't
// have to know about the existence of this extra functionality

class DisplayRepresentor : public Representor {
public:
  DisplayRepresentor(std::ostream &os) : Representor(os) {}
  void visit(String &string);
  void visit(List &list);
};
#endif