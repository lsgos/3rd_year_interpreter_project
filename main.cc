/*
The main loop of the program, defining the entry point for the
application. This is fairly simple as all the heavy lifting is done in
the classes. The main distinction here is whether the program starts
as a REPL, or, if it is passed command line arguments, tries to
open a file and interpret it as a script.

*/
#include <fstream>
#include <iostream>

#include "env.h"
#include "lexer.h"
#include "lisp_exceptions.h"
#include "parser.h"
#include "sexp.h"

/*
If this program is called with no arguments, launch a
read-eval-print-loop, where commands are interpreted and the results
printed interactively
*/

int repl() {
  auto psr = Parser(std::cin);
  GlobalEnv env;
  while (true) {
    try {
      std::cout << " <<=  ";
      auto sexp = psr.read_sexp(env);
      if (!sexp) {
        // EOF character: exit the interpreter
        throw exit_interpreter();
      }
      sexp = sexp->eval(env);
      std::cout << " --> " << *sexp << std::endl;
      env.collect_garbage();
    } catch (exit_interpreter &e) {
      break;
    } catch (std::exception &e) {
      std::cout << e.what() << std::endl;
      std::cin.clear();
      std::cin.ignore(1000, '\n');
    }
  }
  return 0;
}

/*
If the program is called with arguments, the first argument is interpreted
as a filename for the a script, and the script is opened and the results
interpreted. In this mode, results of expressions are
not printed to the screen by default as in the repl and error
reporting includes the location in the file where the error occurred.
If there are more commands in argv, they are made avaliable to the
script as a list of strings in the variable ARGV.
*/

int script(int argc, char *argv[]) {
  char *filename = argv[1];

  std::ifstream file;
  file.open(filename);
  if (!file.is_open()) {
    std::cout << "Couldn't open file " << filename << std::endl;
    return 1;
  }
  auto psr = Parser(file);
  GlobalEnv env;
  env.bind_argv(argc, argv);
  try {

    SExp *exp = psr.read_sexp(env);
    while (file.good()) {

      exp->eval(env);
      env.collect_garbage();
      exp = psr.read_sexp(env);
    }
  } catch (exit_interpreter &e) {
    return 0;
  } catch (std::exception &e) {
    // if an error occurs, report the file and the line so the user can find
    // it easily
    std::cout << "[" << filename << ":" << psr.get_linenum() << ":"
              << psr.get_linepos() << "] " << e.what() << std::endl;
    return 1;
  }
  return 0;
}

int main(int argc, char *argv[]) {

  if (argc == 1) {
    return repl();
  } else {
    return script(argc, argv);
  }
}