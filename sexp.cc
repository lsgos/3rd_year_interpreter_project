#include "sexp.h"
#include "env.h"
#include "lisp_exceptions.h"
#include <list>
#include <memory>
// this inheritance model is quite complicated due to a lot of pointers flying
// around: would using a monolithic 'thing'
// datatype be better?

// this is a helper function to deal with the default behaviour of the unique
// ptr reset function, which does not check if it is trying to reset the pointer
// to the memory it currently points at. We want to be able to do this, so we
// need this helper. Note that this neatly handles garbage collection of the
// evaluated
// structure

bool is_function(LispType type) {
  return (type == LispType::PrimitiveFunction ||
          type == LispType::LambdaFunction);
}

// void maybe_reset(SExp& &unique_ptr, SExp& &new_ptr) {
//  if (unique_ptr.get() != new_ptr.get()) {
//    unique_ptr = std::move(new_ptr); // move ownership
//  } else {
//    // pointer 'own' the same resource; release the new one.
//    unique_ptr.release();
//    unique_ptr = std::move(new_ptr);
//  }
//  return;
//}
//
// simple helper. Note that evaluate replaces a unique pointer to an sexp with
// the value of the sexp it points to.
// void evaluate(SExp& &expression, Env &env) {
//  SExp& evaled = expression->eval(env);
//  if (expression.get() != evaled.get()) {
//    expression = std::move(evaled); // move ownership
//  } else {
//    // pointer 'own' the same resource; release the new one.
//    expression.release();
//    expression = std::move(evaled);
//  }
//  return;
//}
//
// todo: is it clearer to wrap up the functionality of sexp pointers in a class?
// maybe not

// Later, this will return the object named by the atom. Should we return a copy
// or a reference to it??
SExp *Atom::eval(Env &env) { return env.lookup(id); }

SExp *List::eval(Env &env) {
  if (elems.empty()) {
    throw evaluation_error("Cannot evaluate the empty list");
  }
  SExp *head = elems
                   .front()     // first element in the list
                   ->eval(env); // evaluate the expression
  elems.pop_front();

  if (!is_function(head->type())) {
    throw evaluation_error(
        "Expected function"); // TODO make this error less shit
  }
  auto func = static_cast<Function *>(head);
  SExp *result = func->call(elems, env);
  return result;
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
  stream << "<primitive " << fn.get_name() << " >";
}