#ifndef LISP_EXCEPTIONS_H
#define LISP_EXCEPTIONS_H

#include <exception>
#include <string>

// custom exceptions used by lisp.

class parser_exception : public std::exception {
public:
  virtual const char *what() const throw() { return msg.c_str(); }
  parser_exception(int linenum, std::string msg_str = "Parse exception")
      : linenum(linenum) {
    msg = "[Line " + std::to_string(linenum) + "]: " + msg_str;
  }

private:
  std::string msg;
  int linenum;
};

// These should never be thrown: if one of these is encountered, it means
// something has gone wrong somewhere in my logic
class implementation_error : public std::exception {
public:
  virtual const char *what() const throw() { return msg.c_str(); }
  implementation_error(std::string msg = "Implementation error") : msg(msg) {}

private:
  std::string msg;
};

class evaluation_error : public std::exception {
public:
  virtual const char *what() const throw() { return msg.c_str(); }
  evaluation_error(std::string msg = "runtime error") : msg(msg) {}

private:
  std::string msg;
};

class exit_interpreter : public std::exception {
  virtual const char *what() const throw() { return msg.c_str(); }

private:
  std::string msg;

public:
  exit_interpreter() : msg("") {}
};
#endif