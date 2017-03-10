#ifndef HEAP_H
#define HEAP_H

#include "lisp_exceptions.h"
#include <unordered_map>

class Env;
class GlobalEnv;
class SExp;
// The heap class is responsibe for garbage collection, maintaining a list of
// all memory adresses in current use. The most important functions are
// allocate, which wraps calls to new within the lisp world. All memory thus
// allocated is mark-and-sweepable.
class Heap {
private:
  std::unordered_map<SExp *, bool> objects;
  void reset_marks();
  void mark(SExp *);
  void sweep();

public:
  SExp *allocate(SExp *new_object);
  void collect_garbage(Env &env);
  Heap() {}
  Heap(Heap &&other) = default;
  // Move assignment operator.
  Heap &operator=(Heap &&) = default;
  Heap(const Heap &) = delete;
  Heap &operator=(const Heap &) = delete;
  ~Heap();
};

#endif
