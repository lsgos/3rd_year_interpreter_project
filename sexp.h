#ifndef SEXP_H
#define SEXP_H

#include "lisp_exceptions.h"
#include <functional>
#include <iostream>
#include <list>
#include <memory>
#include <string>

class SExp;
class Env;
// typedef std::unique_ptr<SExp> sexp_ptr;

// Alternatively, we could just use shared_ptrs instead?
// Or use const * pointers and delegate garbage collection
// to the Env datatype. Then we can use mark-and-sweep or
// similar.

// Possibly can use references to a managed cell type,
// stored in the env, exclusively? then GC the envs.
// class SexpPtr {
//  std::unique_ptr<SExp> unique_value;
//  SExp *ref_value;
//
// public:
//  void evaluate();
//};
// this is a helper function to deal with the default behaviour of the unique
// ptr reset function, which does not check if it is trying to reset the pointer
// to the memory it currently points at. We want to be able to do this, so we
// need
// this helper
// void maybe_reset(sexp_ptr &unique_ptr, SExp *new_ptr);
// void evaluate(sexp_ptr &expression, Env &env);
// Classes to represent s-expressions.
// The visitor pattern used means that SExps only posseses a single function,
// that returns the address of an
// S-expression. Then visitor classes are used to dispatch functions across the
// data structure.

class Number;
class String;
class Bool;
class Atom;
class List;
class PrimitiveFunction;

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

class SExpVisitor {
public:
  virtual void visit(Number &number) = 0;
  virtual void visit(String &string) = 0;
  virtual void visit(Atom &atom) = 0;
  virtual void visit(Bool &boolean) = 0;
  virtual void visit(List &list) = 0;
  virtual void visit(PrimitiveFunction &fn) = 0;
};

class SExp {
public:
  // a function allowing visitor classes
  virtual void exec(SExpVisitor &visitor) = 0;
  // eval returns a unique_ptr to a data structure representing the value of the
  // SExp. This may be the same as the address of the object for primitive
  // types, or it may be a new data structure, for function calls (lists). The
  // helper function evaluate should be used to handle the pointer swap.
  // TODO: would it be neater to create a unique_ptr wrapper class with these
  // characteristics?
  virtual SExp *eval(Env &env) = 0;
  // returns an enum value giving which subclass of SExp an instantiation of
  // this interface actually is. This is needed when one expects a specific type
  // (e.g numbers for numeric primitives, functions for function application)
  // and one has to downcast to the type from the generic in order to extract
  // its data. Checking with this first preserves sanity
  virtual LispType type() const = 0;
  virtual ~SExp() {}

private:
  // ban the assingnment operator by making it private
  SExp *operator=(const SExp *rhs);
};

class Number : public SExp {
  // numbers eval onto their value
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
  double number;
};

class String : public SExp {
public:
  String(std::string str) : str(str) {}
  ~String() {}
  void exec(SExpVisitor &visitor) override { visitor.visit(*this); }
  std::string val() { return str; }
  virtual SExp *eval(Env &env) override { return this; }
  LispType type() const override { return LispType::String; }

private:
  std::string str;
};

class Atom : public SExp {
public:
  Atom(std::string id) : id(id) {}
  ~Atom() {}
  void exec(SExpVisitor &visitor) override { visitor.visit(*this); }
  std::string get_identifier() { return id; }
  virtual SExp *eval(Env &env) override;
  LispType type() const override { return LispType::Atom; }

private:
  std::string id;
};

class Bool : public SExp {
public:
  Bool(bool val) : value(val) {}
  ~Bool() {}
  void exec(SExpVisitor &visitor) override { visitor.visit(*this); }
  bool val() { return value; }
  virtual SExp *eval(Env &env) override { return this; }
  LispType type() const override { return LispType::Bool; }

private:
  bool value;
};
// List data object. Note that this class behaves as a single object: the actual
// linked list to the s-exps it containts is in List.elems
class List : public SExp {
public:
  void exec(SExpVisitor &visitor) override { visitor.visit(*this); }
  std::list<SExp *> elems;
  virtual SExp *eval(Env &env) override;
  LispType type() const override { return LispType::List; }
  ~List() override {}

  List() {}
};

// interface to represent lisp function objects.
class Function : public SExp {
public:
  // want to pass this by reference, and have the ability to change sexp_ptr to
  // point at a new structure, or
  // take ownership of it. Possibly release it before passing it in? We want
  // ability to mutate and return without
  // copying, as well as to read from and create a new object (e.g sum). Also
  // want to cheaply reference objects
  // saved in the scope: feels wrong that (car l) requires copying the whole of
  // l, taking the car, then throwing
  // away the copy! on the other hand, if we always use references, (cons x l)
  // requires copying c and l in order to
  // leave them both immutable. Alternative is mutable variables, but this
  // doesn't really fit scheme's model well
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

class LambdaFunction : public Function {
private:
  const Env *closure;
  std::list<Atom *> params;
  std::list<SExp *> args;
  std::list<SExp *> body;

public:
  virtual SExp *call(std::list<SExp *> args, Env &env) override;
  LispType type() const override { return LispType::LambdaFunction; }
};

// visitor class, writes a representation of an s-expression to string
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
};

#endif