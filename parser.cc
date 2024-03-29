#include "parser.h"


SExp *Parser::read_sexp(Env &env) {
  auto token = lexer.get_token();
  if (token == Token::eof) {
    return nullptr;
  }
  return parse(env, token);
}
SExp *Parser::parse(Env &env, Token token) {
  switch (token) {
  case Token::open_bracket:
    return parse_list(env);
  case Token::num:
    return env.manage(new Number(lexer.get_parsed_num()));
  case Token::string:
    return env.manage(new String(lexer.get_parsed_str()));
  case Token::atom:
    return env.manage(new Atom(lexer.get_parsed_str()));
  case Token::kw_true:
    return env.manage(new Bool(true));
  case Token::kw_false:
    return env.manage(new Bool(false));
  case Token::kw_quote:
    return mk_quoted_list(env);
  case Token::eof:
    throw parser_error("Unexpected EOF");
  case Token::close_bracket:
    // these shouldn't be encountered outside the list parse function
    throw parser_error("Unexpected )");
  default:
    std::cout << "Found " << int(token) << std::endl;
    throw implementation_error("Non-irrefutable patterns in parser");
  }
}
SExp *Parser::parse_list(Env &env) {
  std::list<SExp *> elems;
  for (auto token = lexer.get_token(); token != Token::close_bracket;
       token = lexer.get_token()) {
    auto elem = parse(env, token);
    elems.push_back(elem);
  }
  return env.manage(new List(elems));
}
// this supports the backtick quote syntactic sugar: '(1 2) is transformed to
// (quote (1 2)) as a macro (i.e before the code is interpreted)
SExp *Parser::mk_quoted_list(Env &env) {
  std::list<SExp *> elems;
  elems.push_back(env.manage(new Atom("quote")));
  elems.push_back(parse(env, lexer.get_token()));
  return env.manage(new List(elems));
}
