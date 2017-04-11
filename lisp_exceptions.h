#ifndef LISP_EXCEPTIONS_H
#define LISP_EXCEPTIONS_H

#include <exception>
#include <string>

// custom exceptions used by lisp.

class lisp_error : public std::exception {
private:
  std::string msg;

public:
  virtual const char *what() const throw() { return msg.c_str(); }
  lisp_error(std::string default_msg, std::string err_msg = "") {
    msg = default_msg + err_msg;
  }
  virtual ~lisp_error() {}
};

class parser_error : public lisp_error {
public:
  parser_error(std::string msg) : lisp_error("Parser error: ", msg) {}
};

class implementation_error : public lisp_error {
public:
  implementation_error(std::string msg)
      : lisp_error("Implementation error: ", msg) {}
};

class evaluation_error : public lisp_error {
public:
  evaluation_error(std::string msg) : lisp_error("Evaluation error: ", msg) {}
};

class io_error : public lisp_error {
public:
  io_error(std::string msg) : lisp_error("IO error: ", msg) {}
};

class exit_interpreter : public lisp_error {
public:
  exit_interpreter() : lisp_error("exit", "") {}
};

#endif