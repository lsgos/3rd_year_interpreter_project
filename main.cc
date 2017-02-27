/*
This is no longer simply the lexer, but now includes large parts of the business
code of an interpreter.
*/
#include <iostream>

// TODO: check if this is the right way to do modules correctly
#include "env.h"
#include "lexer.h"
#include "lisp_exceptions.h"
#include "parser.h"
#include "sexp.h"

int main() {

  {
    auto p = Parser(std::cin);
    GlobalEnv env = GlobalEnv();
    while (true) {
      try {
        std::cout << " >>  ";
        auto sexp = p.read_sexp(env);
        Representor repr = Representor(std::cout);
        sexp = sexp->eval(env);
        sexp->exec(repr);
        std::cout << "\n";
        env.collect_garbage();
      } catch (exit_interpreter &e) {
        break;
      } catch (std::exception &e) {
        //todo: make sure EOF kills the interpreter as well
        std::cout << "Exception: " << e.what() << std::endl;
        std::cin.clear();
        std::cin.ignore(1000,'\n');
      }
    }
  }

  return 0;
}
