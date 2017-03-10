#include "lexer.h"

// TODO lex negative numbers and comments properly

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
  } else if (isdigit(c) || c == '-' || c == '.') {
    return lisp_number(c);
  } else if (is_lisp_symbol(c)) {
    return lisp_atom(c);
  } else if (c == '\"') {
    return lisp_string(c);
  } else if (c == '#') {
    return lisp_bool(c);
  } else if (c == EOF) {
    return Token::eof;
  } else if (c == ';') {
    consume_comment();
    return get_token();
  }
  throw implementation_error("Non-irrefutable logic pattern in lexer");
}
// ignore whitespace characters, keeping track of the internal line number
// counter.
void Lexer::consume_spaces() {
  char c = stream.get();
  while (std::isspace(c)) {
    if (c == newline) {
      ++linenum;
    }
    c = stream.get();
  }
  stream.putback(c);
  return;
}
// called when the comment character ; is encountered: ignore everything until a
// newline
void Lexer::consume_comment() {
  for (char c = stream.get(); c != newline; c = stream.get())
    ;
  ++linenum;
  return;
}
Token Lexer::lisp_number(char c) {
  auto buf = std::stringstream("");
  //- is allowed in numbers, but only as the first character
  if (c == '-') {
    buf.put(c);
    c = stream.get();
  }

  for (; isdigit(c); c = stream.get()) {
    if (c == EOF) {
      throw parser_exception(linenum, "Reached unexpected end-of-file "
                                      "while parsing numeric literal");
    }
    buf.put(c);
  }
  if (c == '.') {
    buf.put('.');
    for (c = stream.get(); isdigit(c); c = stream.get()) {
      if (c == EOF) {
        throw parser_exception(linenum, "Reached unexpected "
                                        "end-of-file while parsing "
                                        "numeric literal");
      }
      buf.put(c);
    }
  }
  if (!(is_delim(c) || c == ')') || buf.str() == "-") {
    // the scheme like behaviour is to accept everything that is not
    // a number as a valid atom (even weird stuff like 3.23214123a
    // is a valid name)
    // to get this behaviour, we just reset the scope of the stream
    // by unloading the buffer, then call list_atom
    buf.put(c);
    std::string atom = buf.str();
    for (auto it = atom.rbegin(); it != atom.rend(); ++it) {
      stream.putback(*it);
    }
    return lisp_atom(stream.get());
  }
  stream.putback(c);
  buf >> parsed_num;
  return Token::num;
}
Token Lexer::lisp_string(char c) {

  auto buf = std::stringstream("");
  for (c = stream.get(); c != '\"'; c = stream.get()) {

    if (c == '\\') {
      // allow escaped special characters, like ", \n, etc
      char esc = stream.get();
      switch (esc) {
      case '\"':
        buf.put(esc);
        break;
      case 'n':
        buf.put('\n');
        break;
      case 't':
        buf.put('\t');
        break;
      case '\'':
        buf.put(esc);
        break;
      case '\\':
        buf.put('\\');
        break;
      default:
        throw parser_exception(linenum,
                               "Unrecognised escape sequence in parser");
      }
      continue;
    }
    if (c == EOF) {
      throw parser_exception(linenum, "Reached unexpected "
                                      "end-of-file: expected "
                                      "closing \"");
    }
    buf.put(c);
  }
  // don't putback- consume second "
  parsed_str = buf.str();
  return Token::string;
}
Token Lexer::lisp_atom(char c) {
  auto buf = std::stringstream("");
  for (; is_lisp_symbol(c) || isdigit(c) || c == '.'; c = stream.get()) {
    buf.put(c);
  }
  stream.putback(c);
  std::string str = buf.str();
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
// valid components of lisp atoms
bool Lexer::is_lisp_symbol(char c) {
  // this is a little bit dumb
  return (isalpha(c) || c == '!' || c == '$' || c == '%' || c == '&' ||
          c == '|' || c == '*' || c == '+' || c == '-' || c == '/' ||
          c == ':' || c == '<' || c == '>' || c == '=' || c == '?' ||
          c == '@' || c == '^' || c == '_' || c == '~');
}
// valid
bool Lexer::is_delim(char c) { return (std::isspace(c) || c == EOF); }
