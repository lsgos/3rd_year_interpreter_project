CXX=clang++
CXXFLAGS= -std=c++11

debug: CXXFLAGS += -DDEBUG -g
debug: build
release: build

build: main.o sexp.o lexer.o parser.o env.o heap.o
	$(CXX) main.o lexer.o sexp.o parser.o env.o heap.o -o main

lexer.o: lisp_exceptions.h lexer.h
sexp.o: lisp_exceptions.h sexp.h
parser.o: lexer.h sexp.h
heap.o: env.h sexp.h
env.o: sexp.h env.h primitives.h
main.o: lexer.o lexer.h sexp.h sexp.o parser.h env.o

format: main.cc lexer.cc lisp_exceptions.h lexer.h sexp.cc sexp.h parser.h parser.cc env.h env.cc
	clang-format -style="llvm" -i main.cc lexer.cc lisp_exceptions.h lexer.h sexp.cc sexp.h parser.h parser.cc env.cc
clean:
	rm *.o main
valgrind: debug
	valgrind --tool=memcheck --leak-check=full ./main
