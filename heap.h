#ifndef HEAP_H
#define HEAP_H

#include "lisp_exceptions.h"
#include <unordered_map>
#include <utility>

class Env;
class GlobalEnv;
class SExp;
/*
The heap class is responsible for garbage collection, maintaining a
record of all memory adresses in current use. The most important
function is manage. This maps calls to new within s-expression
logic. Any object that is created in this way is then managed by the
garbage collector. collect_garbage deletes any memory not in use by
using a mark-and-sweep algorithm from the symbol table to determine
which s-exp objects are still reachable from current environment, and
deleting unreachable memory

*/

class Heap {
private:
  std::unordered_map<SExp *, bool> objects;
  void reset_marks();
  void mark(SExp *);
  void sweep();
  void swap(Heap& a, Heap&b) {
  	std::swap(a.objects, b.objects);
  }
public:
  SExp *manage(SExp *new_object);
  void collect_garbage(Env &env);
  
  Heap() {}
  Heap(Heap &&other);
  // Move assignment operator.
  Heap &operator=(Heap);
  // copying this object is unsafe because they are _uniquely_ responsible for memory
  // of external objects
  Heap(const Heap &) = delete;
  Heap &operator=(const Heap &) = delete;
  ~Heap();
};

#endif
