#include "parser.h"

SExp *Parser::read_sexp(Env &env) { return parse(env, lexer.get_token()); }
SExp *Parser::parse(Env &env, Token token) {
  switch (token) {
  case Token::open_bracket:
    return parse_list(env);
  case Token::num:
    return env.allocate(new Number(lexer.get_parsed_num()));
  case Token::string:
    return env.allocate(new String(lexer.get_parsed_str()));
  case Token::atom:
    return env.allocate(new Atom(lexer.get_parsed_str()));
  case Token::kw_true:
    return env.allocate(new Bool(true));
  case Token::kw_false:
    return env.allocate(new Bool(false));
  case Token::kw_quote:
    return mk_quoted_list(env);
  case Token::eof:
    throw parser_exception(lexer.get_linenum(), "Unexpected EOF");
  default:
    std::cout << "Found " << int(token) << std::endl;
    throw implementation_error("Non-irrefutable patterns in parser");
  }
}
SExp *Parser::parse_list(Env &env) {
  auto list = std::unique_ptr<List>(new List);
  for (auto token = lexer.get_token(); token != Token::close_bracket;
       token = lexer.get_token()) {
    auto elem = parse(env, token);
    list->elems.push_back(elem);
  }
  auto ptr = list.release();
  return env.allocate(static_cast<SExp *>(ptr));
}
// this supports the backtick quote syntactic sugar: '(1 2) is transformed to
// (quote (1 2))
SExp *Parser::mk_quoted_list(Env &env) {
  auto list = new List;
  list->elems.push_back(env.allocate(new Atom("quote")));
  list->elems.push_back(parse(env, lexer.get_token()));
  return env.allocate(list);
}
