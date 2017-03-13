/*
The main loop of the program, defining the entry point for the application.
This is fairly simple as all the heavy lifting is done in the classes.
*/
#include <fstream>
#include <iostream>

#include "env.h"
#include "lexer.h"
#include "lisp_exceptions.h"
#include "parser.h"
#include "sexp.h"

// TODO attempt to remove all duplicated includes from the header files

int repl() {
  auto p = Parser(std::cin);
  GlobalEnv env = GlobalEnv();
  while (true) {
    try {
      std::cout << " <<=  ";
      auto sexp = p.read_sexp(env); // should we use the stream extraction
      // operator here? maybe not, since we want
      // to catch exceptions thrown by the
      // parser, and canonically the stream extractor should set the failbit
      // rather than throw
      sexp = sexp->eval(env);
      std::cout << " --> " << *sexp << std::endl;
      env.collect_garbage();
    } catch (exit_interpreter &e) {
      break;
    } catch (std::exception &e) {
      // TODO: make sure EOF kills the interpreter as
      // well
      std::cout << "Exception: " << e.what() << std::endl;
      std::cin.clear();
      std::cin.ignore(1000, '\n');
    }
  }
  return 0;
}

int script(char *filename) {

  std::ifstream file;
  file.open(filename);
  if (!file.is_open()) {
    std::cout << "Couldn't open file " << filename << std::endl;
    return 1;
  }
  auto p = Parser(file);
  GlobalEnv env = GlobalEnv();
  SExp *exp = p.read_sexp(env);
  while (file.good()) {
    try {
      exp->eval(env);
      env.collect_garbage();
      exp = p.read_sexp(env);

    } catch (exit_interpreter &e) {
      return 0;
    } catch (std::exception &e) {
      // TODO: make sure EOF kills the interpreter as
      // well
      std::cout << "Exception: " << e.what() << std::endl;
      return 1;
    }
  }
  return 0;
}

int main(int argc, char *argv[]) {
  if (argc == 1) {
    return repl();
  } else if (argc == 2) {
    return script(argv[1]);
  } else {
    std::cout << "Please provide only one or two arguments" << std::endl;
    // todo: pass the rest into the program as parameters?
    return 1;
  }
}