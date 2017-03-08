#ifndef ENV_H
#define ENV_H
#include "lisp_exceptions.h"
#include "heap.h"
//#include "sexp.h"
#include <functional>
#include <list>
#include <string>
#include <vector>

/*
The Env manages scope resolution and definition via a symbol table. This
functionality is essentially a wrapper around a hash table mapping strings to
SExp pointers.
The GlobalEnv also contains a heap object, and thus reponsible for tracking memory.
Env objects can also be used to track memory, but they simply send it on to the
global scope.

Global Env's and heaps only really make sense if they are unique (one per runtime
instance) so their copy constructors and assignment operators are deleted.
In practice we can just not use them but it adds semantic clarity.
*/

class Env {
  private:
  //may have to make Env an abstract class: think the attempt to inherit this field is causing us problems
   GlobalEnv* const global;

  protected:
    std::unordered_map < std::string, SExp* > scope;
    //this should never be used except by inheritin classes
    Env() : global(nullptr) {}
  public:
    virtual Env capture_scope() {
      //if the scope is not global, just provide a copy.
      return *this;
    }
    virtual SExp* allocate(SExp* obj);
    SExp *lookup(std::string id);
    void def(std::string id, SExp* );
    //TODO finish this crud

    Env(GlobalEnv& g);
    virtual ~Env() {}
    friend class Heap;
};


class GlobalEnv : public Env {
private:
  Heap heap;
  //helper functions for constructing builtin function objects
  SExp *mk_numeric_primitive(std::function<double(double acc, double x)> func,
                             std::string funcname);

  SExp *mk_builtin(std::function<SExp* (std::list<SExp*>, Env&)>, std::string name);
public:
  // lookup the value with name id and put it in p if it exists
  GlobalEnv();
  Env capture_scope() override;
  SExp *allocate(SExp *obj) override { return heap.allocate(obj); }
  void bind_primitives();
  ~GlobalEnv() override;
   GlobalEnv(GlobalEnv&& other) = default;
   // Move assignment operator.
   GlobalEnv& operator=(GlobalEnv&&) = default;
   GlobalEnv(const GlobalEnv&) = delete;
   GlobalEnv& operator = (const GlobalEnv&) = delete;
   void collect_garbage() {heap.collect_garbage(*this);}
   //friend class Heap;
};


#endif
