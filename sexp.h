#ifndef SEXP_H
#define SEXP_H

#include "env.h"
#include <functional>
#include <iostream>
#include <list>
#include <memory>
#include <string>
#include <unordered_map>
// This header file is the core of the language: this defines the
// datatypes of the language. All valid expressions are s-expressions,
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

enum class LispType {
  Number,
  String,
  Bool,
  Atom,
  List,
  PrimitiveFunction,
  LambdaFunction,
};

bool is_function(LispType type);

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
  // returns an enum value giving which subclass of SExp an instantiation of
  // this interface actually is. This is needed when one expects a specific type
  // (e.g numbers for numeric primitives, functions for function application)
  // and one has to downcast to the type from the generic in order to extract
  // its data. Checking with this first preserves safety
  virtual LispType type() const = 0;
  virtual ~SExp() {}
};

// Only floating point numbers are provided. This simplifies implementation
// but is used in mainstream, useful languages like Lua and Javascript.
// Numbers are their own values
class Number : public SExp {
public:
  Number(double number) : number(number) {}
  ~Number() override {}
  void exec(SExpVisitor &visitor) override { visitor.visit(*this); }
  double val() { return number; }
  LispType type() const override { return LispType::Number; }
  // Numbers are primitive, so they are their own values. Similarly for all
  // types except lists
  virtual SExp *eval(Env &env) override { return this; }

private:
  const double number;
};
// string datatype, basically the same as number, a primitive type.
// Like numbers, strings are their own values.
class String : public SExp {
public:
  String(std::string str) : str(str) {}
  ~String() {}
  void exec(SExpVisitor &visitor) override { visitor.visit(*this); }
  std::string val() { return str; }
  virtual SExp *eval(Env &env) override { return this; }
  LispType type() const override { return LispType::String; }

private:
  const std::string str;
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

// Booleans, another primitve type. See above.
class Bool : public SExp {
public:
  Bool(bool val) : value(val) {}
  ~Bool() {}
  void exec(SExpVisitor &visitor) override { visitor.visit(*this); }
  bool val() { return value; }
  virtual SExp *eval(Env &env) override { return this; }
  LispType type() const override { return LispType::Bool; }

private:
  const bool value;
};
// Linked lists. Lists are eval'ed as function calls, of the form
//(f arg1 arg2 ...)
class List : public SExp {
public:
  List(std::list<SExp* > list) : elems(list) {}
  void exec(SExpVisitor &visitor) override { visitor.visit(*this); }
  const std::list<SExp *> elems; //TODO fix this so this can be declared const: maybe add a new constructor? 
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
  // constructor: caputures a scope, a list of parameter names and a body, a
  // sequence of SExps to execute when the function is called.
  LambdaFunction(Env env, std::list<std::string> params, std::list<SExp *> body)
      : closure(env), params(params), body(body) {}
  virtual SExp *call(std::list<SExp *> args, Env &env) override;
  LispType type() const override { return LispType::LambdaFunction; }
  void exec(SExpVisitor &visitor) override { visitor.visit(*this); }
  SExp *eval(Env &env) override { return this; }
  ~LambdaFunction() override {}
  friend class Heap; // needs to access the env and body of lambdas for garbage
                     // collection
  friend class Representor;
};

// visitor class, writes a representation of an s-expression to a stream
class Representor : public SExpVisitor {
private:
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
};

#endif