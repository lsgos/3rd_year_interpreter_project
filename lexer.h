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

The lexer tracks the current line and position in the file it's
reading, which is used to provide more helpful error reporting. It's
only approximatly accurate for non-parser errors. 
*/



/*
Lexer tokens represent the distinct syntactic classes we can encounter in our input.
*/
enum class Token {
  eof,
  open_bracket,
  close_bracket,
  kw_quote,		//' backtick syntactic sugar
  kw_true,		//#t
  kw_false,		//#f
  atom,			// atomic identifier
  num,			// double
  string,		// string literals
};


class Lexer {
public:
  std::istream &stream;
  std::string parsed_str;
  double parsed_num;
  
  Lexer(std::istream &stream) : stream(stream), linenum(1), linepos(1) {}
  
  //return the last string or numeric literal parsed by the lexer
  std::string get_parsed_str() { return parsed_str; }
  double get_parsed_num() { return parsed_num; }
  int get_linenum() { return linenum; }
  int get_linepos() { return linepos; }
  
  //return the next token from the stream the lexer is processing
  Token get_token();

private:
  int linenum; //current line number of the stream
  int linepos; //current position in the line 
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