#ifndef ENV_H
#define ENV_H
#include "heap.h"
#include "lisp_exceptions.h"
//#include "sexp.h"
#include <functional>
#include <list>
#include <string>
#include <vector>

/*
The Env manages scope resolution and definition via a symbol table. This
functionality is essentially a wrapper around a hash table mapping strings to
SExp pointers.
The GlobalEnv also contains a heap object, which manages memory
*/

class Env {
private:

  GlobalEnv *const global;

protected:
  std::unordered_map<std::string, SExp *> scope;
  // this should never be used except by inheritin classes
  Env() : global(nullptr) {}

public:
  virtual Env capture_scope() {
    // if the scope is not global, just provide a copy.
    return *this;
  }
  virtual SExp *allocate(SExp *obj);
  SExp *lookup(std::string id);
  void def(std::string id, SExp *);

  Env(GlobalEnv &g);
  virtual ~Env() {}
  friend class Heap;
};

class GlobalEnv : public Env {
private:
  Heap heap;
  // helper functions for constructing builtin function objects
  SExp *mk_numeric_primitive(std::function<double(double acc, double x)> func,
                             std::string funcname);

  SExp *mk_builtin(std::function<SExp *(std::list<SExp *>, Env &)>,
                   std::string name);

public:
  // lookup the value with name id and put it in p if it exists
  GlobalEnv();
  Env capture_scope() override;
  SExp *allocate(SExp *obj) override { return heap.allocate(obj); }
  void bind_primitives();
  ~GlobalEnv() override;
  void bind_argv(int argc, char *argv[]);
  /*
  the global env cannot be moved without breaking
  everything (since the dispatch of allocation
  to the garbage collector depends on a pointer to
  it being stored in Env's that are created using it: see env.cc).
  As a result, all constructors and operators that would allow it
  to be moved or copied are forbidden
  */
  GlobalEnv(GlobalEnv &&other) = delete;
  GlobalEnv &operator=(GlobalEnv &&) = delete;
  GlobalEnv(const GlobalEnv &) = delete;
  GlobalEnv &operator=(const GlobalEnv &) = delete;
  void collect_garbage() { heap.collect_garbage(*this); }
};

#endif
