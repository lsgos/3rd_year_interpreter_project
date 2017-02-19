//A simulated heap to keep track of allocated memory
#ifndef HEAP_H
#define HEAP_H

#include "sexp.h"
#include <list>
#include <memory>

class Cell {
    private:
        bool marked;
        std::unique_ptr<SExp> cell;
    public:
        Cell(SExp* ptr) :  cell(ptr) {}
        Cell() : marked(false), cell(nullptr){}
        // Move constructor.  
        Cell(Cell&& other) = default;
        // Move assignment operator.  
        Cell& operator=(Cell&&) = default;
        Cell(const Cell&) = delete;
        Cell& operator = (const Cell&) = delete;
        void mark();
        void reset();
        SExp* get_ref();
};
class Heap {
    private:
    std::list<Cell> objects;
    public:
    SExp*  allocate(SExp* new_object);
    Heap() {}
    Heap(Heap&& other) = default;
    // Move assignment operator.  
    Heap& operator=(Heap&&) = default;
    Heap(const Heap&) = delete;
    Heap& operator = (const Heap&) = delete;
};

#endif
