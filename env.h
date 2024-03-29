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
  Env() : global(nullptr) {}

public:
  //return a copy of the current symbol table
  virtual Env capture_scope() {
    // if the scope is not global, just provide a copy.
    return *this;
  }
  //Manage a new object with the garbage collector.
  virtual SExp *manage(SExp *obj);
  
  //look up an identifier in the symbol table
  SExp *lookup(std::string id);
  //add a new entry to the symbol table
  void def(std::string id, SExp *);

  Env(GlobalEnv &g);
  virtual ~Env() {}
  friend class Heap;
};

class GlobalEnv : public Env {
private:
  Heap heap;
  //helper functions for creating builtins.
  SExp *mk_numeric_primitive(std::function<double(double acc, double x)> func,
                             std::string funcname);

  SExp *mk_builtin(std::function<SExp *(std::list<SExp *>, Env &)>,
                   std::string name);

public:

  GlobalEnv();
  Env capture_scope() override;
  SExp *manage(SExp *obj) override { return heap.manage(obj); }
  
  //bind the language builtin functions to the symbol table
  void bind_primitives();
  ~GlobalEnv() override;
  void bind_argv(int argc, char *argv[]);
  /*
  the global env cannot be moved without breaking
  everything.
  As a result, all constructors and operators that would allow it
  to be moved or copied are forbidden
  */
  GlobalEnv(GlobalEnv &&other) = delete;
  GlobalEnv &operator=(GlobalEnv &&) = delete;
  GlobalEnv(const GlobalEnv &) = delete;
  GlobalEnv &operator=(const GlobalEnv &) = delete;
  //run the garbage collector
  void collect_garbage() { heap.collect_garbage(*this); }
};

#endif
