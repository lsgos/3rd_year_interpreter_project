#ifndef ENV_H
#define ENV_H

#include "heap.h"
#include "sexp.h"
#include <functional>
#include <list>
#include <string>
#include <unordered_map>
class Env;



class Heap {
    private:
    std::unordered_map<SExp*, bool> objects;
    void reset_marks();
    void mark(SExp*);
    void sweep();
    public:
    SExp*  allocate(SExp* new_object);
    void collect_garbage(Env& env);
    Heap() {}
    Heap(Heap&& other) = default;
    // Move assignment operator.  
    Heap& operator=(Heap&&) = default;
    Heap(const Heap&) = delete;
    Heap& operator = (const Heap&) = delete;
    ~Heap();
};

class Env {
private:
  // this will be used to store the values in a namespace
  std::unordered_map<std::string, SExp* > scope;
  // possible this should be a shared_ptr: otherwise not totally sure how to
  // ensure closures don't outlive their enclosing scopes
  Env *enclosing_scope;
  Heap heap;
  SExp *mk_numeric_primitive(std::function<double(double acc, double x)> func,
                             std::string funcname);
  SExp *mk_cons();
  SExp *mk_quote();
  SExp *mk_define();
  SExp *mk_car();

public:
  // lookup the value with name id and put it in p if it exists
  Env();
  Env(Env *a);
  // bool get_definition(std::string id, sexp_ptr p);
  // now here's an interesting problem: if we stick to our model of building
  // sexps then destructively updating them
  // in order to eval, we need to copy the structures from memory when we
  // lookup a variable. Is there a better way?
  // todo: implement copy constructor on non-trivial sexps
  // p.reset(new_sexp_ptr to copied memory); return true;
  SExp *lookup(std::string id);
  void def(std::string id, SExp *);
  SExp *allocate(SExp *obj) { return heap.allocate(obj); }
  void bind_primitives();
  ~Env();
   Env(Env&& other) = default;
   // Move assignment operator.  
   Env& operator=(Env&&) = default;
   Env(const Env&) = delete;
   Env& operator = (const Env&) = delete;
   void collect_garbage() {heap.collect_garbage(*this);}
   friend class Heap;
};


#endif