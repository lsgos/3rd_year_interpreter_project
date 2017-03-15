#include "heap.h"
#include "env.h"
#include "sexp.h"

SExp *Heap::allocate(SExp *new_object) {
  // objects.push_back(Cell(new_object));
  std::pair<SExp *, bool> entry(new_object, false);
  // std::cout << "Storing " << new_object << std::endl;
  objects.insert(entry);
  return new_object;
}

// The heap class is responsible for manageing the memory usage of
// the program, so it needs to clean up the pointers.
Heap::~Heap() {
  for (auto it = objects.begin(); it != objects.end(); ++it) {
    // std::cout << "Freeing " << it->first << std::endl;
    delete it->first;
  }
}

// setup the heap for mark and sweep: set all the usage marks to zero
void Heap::reset_marks() {
  for (auto it = objects.begin(); it != objects.end(); ++it) {
    it->second = false;
  }
}
// sweep memory, cleaning up anything marked for deletion.
void Heap::sweep() {
  for (auto it = objects.begin(), next = objects.begin(); it != objects.end();
       it = next) {
    next = it;
    ++next;

    if (!(it->second)) {
      SExp *ptr = it->first; // cleanup memory managed by this key
      objects.erase(it);
      // std::cout << "Freeing " << ptr << std::endl;
      delete ptr; // remove from the heap
    }
  }
}

void Heap::mark(SExp *addr) {
  auto entry = objects.find(addr);
  if (entry == objects.end()) {
    throw implementation_error(
        "This shouldn't happen: encountered unmanaged address");
  }
  // mark memory as used
  if (entry->second) {
    // if this is true, we have already marked this object, and have no need
    // to mark it again: return early to avoid infinite cycles
    return;
  }
  entry->second = true;
  // std::cout << "Marked " << entry->first << " as in use" << std::endl;
  auto expr_type = entry->first->type();
  // Lists and functions can contain references to other objects: we need
  // to
  // recursively trace out their tree.

  if (expr_type == LispType::List) {
    auto list = static_cast<List *>(addr);
    for (auto it = list->elems.begin(); it != list->elems.end(); ++it) {
      // mark all sub elements as well
      mark(*it);
    }
  }
  if (expr_type == LispType::LambdaFunction) {
    auto lambda = static_cast<LambdaFunction *>(addr);
    // mark the expressions in the function body
    for (auto obj = lambda->body.begin(); obj != lambda->body.end(); ++obj) {
      mark(*obj);
    }
    // iterate through the closure's bindings, marking the scope
    // captured
    // from
    // it's creation
    for (auto obj = lambda->closure.scope.begin();
         obj != lambda->closure.scope.end(); ++obj) {
      mark(obj->second);
    }
  }
  return;
}

// naive mark-and-sweep: take the environment bindings, and walk through the
// objects described by their bindings, marking everything reachable from the
// root nodes as in use. Then iterate through the object map freeing everything
// that wasn't reachable; these things have no bindings to the rest of the
// program
// and are thus garbage.
void Heap::collect_garbage(Env &env) {
  reset_marks();
  for (auto entry = env.scope.begin(); entry != env.scope.end(); ++entry) {
    mark(entry->second);
  }
  sweep();
}
