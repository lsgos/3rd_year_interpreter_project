#ifndef PARSER_H
#define PARSER_H

#include "env.h"
#include "iostream"
#include "lexer.h"
#include "lisp_exceptions.h"
#include "sexp.h"

class Parser {
public:
  Parser(std::istream &instream) : lexer(Lexer(instream)) {}
  SExp *read_sexp(Env &env);

private:
  SExp *parse(Env &env, Token token);

private:
  Lexer lexer;
  SExp *parse_list(Env &env);
  // this supports the backtick quote syntactic sugar: '(1 2) is transformed to
  // (quote (1 2))
  SExp *mk_quoted_list(Env &env);
};

#endif