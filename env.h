#ifndef ENV_H
#define ENV_H

#include "sexp.h"
#include <functional>
#include <list>
#include <string>
#include <unordered_map>
#include <vector> 

//The Env manages scope resolution and definition via a symbol table, and the Heap tracks 
//and garbage collects resources created by the language runtime. Primitive language 
//builtins are added to the symbol table by the bind_primitives command. 
class Env;
class GlobalEnv;


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
  //may have to make Env an abstract class: think the attempt to inherit this field is causing us problems
   GlobalEnv* const global;
   
  protected:
    std::vector < std::unordered_map < std::string, SExp* > > scope;
    //this should never be used except by inheritin classes 
    Env() : global(nullptr) {}
  public:
    void exit_scope();
    void new_scope();
    virtual Env capture_scope() {
      //if the scope is not global, just provide a copy. 
      return *this;
    }
    virtual SExp* allocate(SExp* obj);
    SExp *lookup(std::string id);
    void bind(std::string id, SExp *);
    virtual void def(std::string id, SExp* ) {
      throw evaluation_error("Illegal define in non-global scope");
    }
    //TODO finish this crud
    
    Env(GlobalEnv& g);
    virtual ~Env() {}
    friend class Heap;
};


class GlobalEnv : public Env {
private:

  //TODO make a seperate scope class to handle lexical scopes, rather than using a pointer to another env?
  //now the env is responsible for the memory there can only really be one at a time (?) or if it would 
  //work recursively it's more sensible to just have one. 
  // this will be used to store the values in a namespace
  // possible this should be a shared_ptr: otherwise not totally sure how to
  // ensure closures don't outlive their enclosing scopes
  Heap heap;
  SExp *mk_numeric_primitive(std::function<double(double acc, double x)> func,
                             std::string funcname);
  SExp *mk_cons();
  SExp *mk_quote();
  SExp *mk_define();
  SExp *mk_car();

public:
  // lookup the value with name id and put it in p if it exists
  GlobalEnv();
  //Env capture_scope() override;
  void def(std::string id, SExp *) override;
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