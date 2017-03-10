/*
The main loop of the program, defining the entry point for the application.
This is fairly simple as all the heavy lifting is done in the classes.
*/
#include <iostream>

#include "env.h"
#include "lexer.h"
#include "lisp_exceptions.h"
#include "parser.h"
#include "sexp.h"

// TODO attempt to remove all duplicated includes from the header files

int main() {

  {
    auto p = Parser(std::cin);
    GlobalEnv env = GlobalEnv();
    while (true) {
      try {
        std::cout << " >>  ";
        auto sexp = p.read_sexp(env); // should we use the stream extraction
        // operator here? maybe not, since we want
        // to catch exceptions thrown by the
        // parser, and canonically the stream extractor should set the failbit
        // rather than throw
        sexp = sexp->eval(env);
        std::cout << *sexp << std::endl;
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
  }

  return 0;
}
