#include "env.h"
#include "heap.h"
#include "sexp.h"
#include <typeinfo>

Heap::Heap(Heap &&other) { objects = std::move(other.objects); }
Heap &Heap::operator=(Heap other) {
  swap(*this, other);
  return *this;
}

// Should be used as heap.allocate(new some_object);
// adds the newly created objects address to the heap's record,
// marking it as unused by default
SExp *Heap::allocate(SExp *new_object) {
  objects[new_object] = false;
  return new_object;
}

// The heap class is responsible for manageing the memory usage of
// the program, so it's destructor must clean up all the memory it
// was responsible for
Heap::~Heap() {
  for (auto it = objects.begin(); it != objects.end(); ++it) {
    delete it->first;
  }
}

// First phase of mark and sweep: mark everything as unused
void Heap::reset_marks() {
  for (auto it = objects.begin(); it != objects.end(); ++it) {
    it->second = false;
  }
}

// sweep memory, cleaning up anything marked for deletion.
void Heap::sweep() {
  // the use of it and next is to avoid invalidating it when we remove it
  // from the table
  for (auto it = objects.begin(), next = objects.begin(); it != objects.end();
       it = next) {
    next = it;
    ++next;

    if (!(it->second)) {
      SExp *ptr = it->first; // get address
      objects.erase(it);     // remove entry from object table
      delete ptr;            // cleanup memory
    }
  }
}

// mark an object and any objects it contains pointers to as reachable
void Heap::mark(SExp *addr) {
  auto entry = objects.find(addr);
  if (entry == objects.end()) {
    // this should never happen
    throw implementation_error(
        "Garbage collector encountered unmanaged address");
  }
  if (entry->second) {
    // if the object is already marked, avoid cycles
    return;
  }
  entry->second = true; // mark address as in use

  // Lists and functions can contain references to other objects: we need
  // to mark the objects they reference as in use as well

  if (typeid(*addr) == typeid(List)) {
    auto list = static_cast<List *>(addr);
    for (auto it = list->elems.begin(); it != list->elems.end(); ++it) {
      mark(*it);
    }
  }
  if (typeid(*addr) == typeid(LambdaFunction)) {
    auto lambda = static_cast<LambdaFunction *>(addr);
    // mark the expressions in the function body
    for (auto obj = lambda->body.begin(); obj != lambda->body.end(); ++obj) {
      mark(*obj);
    }
    // mark the expressions pointed to by the functions closure
    for (auto obj = lambda->closure.scope.begin();
         obj != lambda->closure.scope.end(); ++obj) {
      mark(obj->second);
    }
  }
  return;
}

// naive mark-and-sweep: mark all objects pointed to by names in the symbol
// table as in use, then collect all unmarked memory managed by the heap.

void Heap::collect_garbage(Env &env) {
  reset_marks();
  for (auto entry = env.scope.begin(); entry != env.scope.end(); ++entry) {
    mark(entry->second);
  }
  sweep();
}
