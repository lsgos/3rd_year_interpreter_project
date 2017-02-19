/*
This is no longer simply the lexer, but now includes large parts of the business
code of an interpreter.
*/
#include <cctype>
#include <iostream>
#include <map>
#include <memory>
#include <sstream>
#include <string>
#include <unordered_map>

// TODO: check if this is the right way to do modules correctly
#include "env.h"
#include "lexer.h"
#include "lisp_exceptions.h"
#include "parser.h"
#include "sexp.h"

int main() {

  {
    auto p = Parser(std::cin);
    Env env = Env();
    while (true) {
      try {
        auto sexp = p.read_sexp(env);
        Representor repr = Representor(std::cout);
        sexp = sexp->eval(env);
        sexp->exec(repr);
        std::cout << "\n";
        env.collect_garbage();
      } catch (std::exception &e) {
        std::cout << "Exception: " << e.what() << std::endl;
        break;
      }
    }
  }

  return 0;
}
