#include "heap.h"


void Cell::mark() {
    marked = true;
} 

void Cell::reset() {
    marked = false;
}

SExp* Cell::get_ref() {
    return cell.get();
}

SExp*  Heap::allocate(SExp* new_object) {
    objects.push_back(Cell(new_object));
    return new_object;
}

