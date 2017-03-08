#ifndef LEXER_H
#define LEXER_H

#include "lisp_exceptions.h"
#include <cctype>
#include <iostream>
#include <sstream>
#include <string>

/*
The lexer transforms a stream of characters into a sequence of tokens,
represented by the token enum, that act as an input to the parser.
The tokens represent components of the grammer of the language.
In the cases where the token has an associated unique value, such
as a number or a string, this is stored in the lexer, and must
be retrieved from it using the accessor functions by the parser
when it encounters a appropriate symbol.
*/
enum class Token {
  // lexer tokens: the things that we can encounter in our language.
  eof,
  open_bracket,
  close_bracket,
  kw_quote, //' backtick syntactic sugar
  kw_true,
  kw_false,
  // possibly add more tokens here? should keywords like if, else etc be
  // functions or keywords?
  atom,   // atomic identifier
  num,    // double
  string, // string literals
};

class Lexer {
public:
  std::istream &stream;
  std::string parsed_str;
  double parsed_num;
  int linenum;
  Lexer(std::istream &stream) : stream(stream) { linenum = 1; }
  std::string get_parsed_str() { return parsed_str; }
  double get_parsed_num() { return parsed_num; }
  int get_linenum() { return linenum; }
  Token get_token();

private:
  void consume_comment();
  void consume_spaces();
  Token lisp_number(char c);
  Token lisp_string(char c);
  Token lisp_atom(char c);
  Token lisp_bool(char c);
  bool is_lisp_symbol(char c);
  bool is_delim(char c);
};

#endif