#ifndef LEXER_H
#define LEXER_H

#include <iostream>
#include <string>

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
  Lexer(std::istream &stream) : stream(stream) { linenum = 0; }
  std::string get_parsed_str() { return parsed_str; }
  double get_parsed_num() { return parsed_num; }
  int get_linenum() { return linenum; }
  Token get_token();

private:
  void consume_spaces();
  Token lisp_number(char c);
  Token lisp_string(char c);
  Token lisp_atom(char c);
  Token lisp_bool(char c);
  bool is_lisp_symbol(char c);
  bool is_delim(char c);
};

#endif