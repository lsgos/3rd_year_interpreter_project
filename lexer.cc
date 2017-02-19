#include "lexer.h"
#include "lisp_exceptions.h"

#include <sstream>
#include <string>

const char newline('\n'); // platform dependent

Token Lexer::get_token() {
  consume_spaces();
  auto c = stream.get();
  if (c == '(') {
    return Token::open_bracket;
  } else if (c == ')') {
    return Token::close_bracket;
  } else if (c == '\'') {
    return Token::kw_quote;
  } else if (isdigit(c)) {
    return lisp_number(c);
  } else if (is_lisp_symbol(c)) {
    return lisp_atom(c);
  } else if (c == '\"') {
    return lisp_string(c);
  } else if (c == '#') {
    return lisp_bool(c);
  } else if (c == EOF) {
    return Token::eof;
  }
  throw implementation_error("Non-irrefutable logic pattern in lexer");
}

void Lexer::consume_spaces() {
  char c = stream.get();
  while (c == ' ' || c == newline) {
    if (c == newline) {
      ++linenum;
    }
    c = stream.get();
  }
  stream.putback(c);
  return;
}
Token Lexer::lisp_number(char c) {
  // todo: deal with negative numbers
  auto buf = std::stringstream("");
  for (; isdigit(c); c = stream.get()) {
    if (c == EOF) {
      throw parser_exception(
          linenum,
          "Reached unexpected end-of-file while parsing numeric literal");
    }
    buf.put(c);
  }
  if (c == '.') {
    buf.put('.');
    for (c = stream.get(); isdigit(c); c = stream.get()) {
      if (c == EOF) {
        throw parser_exception(
            linenum,
            "Reached unexpected end-of-file while parsing numeric literal");
      }
      buf.put(c);
    }
  }
  if (!(is_delim(c) || c == ')')) {
    throw parser_exception(
        linenum, "Found unexpected character parsing numeric literal");
  }
  stream.putback(c);
  buf >> parsed_num;
  return Token::num;
}
Token Lexer::lisp_string(char c) {
  // c == "

  auto buf = std::stringstream("");
  for (c = stream.get(); c != '\"'; c = stream.get()) {
    if (c == EOF) {
      throw parser_exception(
          linenum, "Reached unexpected end-of-file: expected closing \"");
    }
    buf.put(c);
  }
  // don't putback- consume second "
  parsed_str = buf.str();
  return Token::string;
}
Token Lexer::lisp_atom(char c) {
  auto buf = std::stringstream("");
  for (; is_lisp_symbol(c) || isdigit(c); c = stream.get()) {
    buf.put(c);
  }
  stream.putback(c);
  std::string str = buf.str();
  // think we can treat all of these as ordinary functions rather than doing
  // pattern matching.
  // if (str == "define") {
  //  return Token::kw_define;
  //} else if (str == "lambda") {
  //  return Token::kw_lambda;
  //} else if (str == "if") {
  //  return Token::kw_if;
  //} else if (str == "else") {
  //  return Token::kw_else;
  //} else if (str == "cond") {
  //  return Token::kw_cond;
  //} else {
  //  parsed_str = str;
  //  return Token::atom;
  //}
  parsed_str = str;
  return Token::atom;
}
Token Lexer::lisp_bool(char c) {
  int b = stream.get();
  char peek = stream.peek();
  if (!(is_delim(peek) || peek == ')')) {
    throw parser_exception(linenum,
                           "Unexpected character following #: expected t or f");
  }
  switch (b) {
  case 't':
    return Token::kw_true;
  case 'f':
    return Token::kw_false;
  default:
    throw parser_exception(linenum,
                           "Unexpected character following #: expected t or f");
  }
}
bool Lexer::is_lisp_symbol(char c) {
  //"!$%&|*+-/:<=?@^_~"
  // this is a little bit dumb
  return (isalpha(c) || c == '!' || c == '$' || c == '%' || c == '&' ||
          c == '|' || c == '*' || c == '+' || c == '-' || c == '/' ||
          c == ':' || c == '<' || c == '>' || c == '=' || c == '?' ||
          c == '@' || c == '^' || c == '_' || c == '~');
}
bool Lexer::is_delim(char c) {
  return (c == ' ' || c == '\n' || c == '\r' || c == EOF);
}
