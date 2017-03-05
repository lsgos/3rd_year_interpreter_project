#include "sexp.h"
#include "env.h"
#include "lisp_exceptions.h"
#include <list>
#include <memory>
#include <sstream>

// this returns false if exp = #f, true else
bool is_true(SExp *exp) {
        return !(exp->type() == LispType::Bool &&
                 static_cast<Bool *>(exp)->val() == false);
}

bool is_function(LispType type) {
        return (type == LispType::PrimitiveFunction ||
                type == LispType::LambdaFunction);
}

SExp *Atom::eval(Env &env) {
        auto value = env.lookup(id);
        if (value != nullptr) {
                return value;
        } else {
                throw evaluation_error("Encountered undefined atom " + id);
        }
}

SExp *List::eval(Env &env) {
        if (elems.empty()) {
                throw evaluation_error("Cannot evaluate the empty list");
        }
        SExp *head = elems
                         .front()     // first element in the list
                         ->eval(env); // evaluate the expression
        auto args = elems;            // copy elems to avoid mutating the list
        args.pop_front();

        if (!is_function(head->type())) {
                throw evaluation_error(
                    "Expected function"); // TODO make this error less shit
        }
        auto func = static_cast<Function *>(head);
        SExp *result = func->call(args, env);
        return result;
}

SExp *LambdaFunction::call(std::list<SExp *> args, Env &env) {
        // check the argument list matches the params of the function
        if (args.size() != params.size()) {

                std::stringstream msg;
                auto repr = Representor(msg);
                msg << "Found mismatched argument list in function ";
                this->exec(repr); // print function name to msg string
                msg << ", Expected " << params.size() << ", found "
                    << args.size();
                throw evaluation_error(msg.str());
        }
        auto f_env = closure; // create a copy of the closure to use when
                              // evaluating the body
        // arg list matches message: go through the list of provided argument,
        // evaluating each one and binding the result to the closure.
        auto arg = args.begin();
        for (auto par = params.begin(); par != params.end(); ++par, ++arg) {
                (*arg) = (*arg)->eval(env);
                f_env.def(*par, *arg);
        }
        SExp *result;
        // evaluate the body of the function, returning the result of the last
        // expression
        // note that the function body is evaluated in the scope captured by the
        // lambda function,
        // not the calling scope.
        for (auto it = body.begin(); it != body.end(); ++it) {
                result = (*it)->eval(f_env);
        }
        return result;
}

void Representor::visit(Number &number) { stream << number.val(); }
void Representor::visit(String &string) {
        stream << "\"";
        stream << string.val();
        stream << "\"";
}
void Representor::visit(Bool &boolean) {
        bool v = boolean.val();
        if (v) {
                stream << "#t";
        } else {
                stream << "#f";
        }
}
void Representor::visit(Atom &atom) { stream << atom.get_identifier(); }
void Representor::visit(List &list) {
        stream << "(";
        if (!list.elems.empty()) {
                list.elems.front()->exec(*this);
                // void representor on all elements in the list.
                for (auto x = ++list.elems.begin(); x != list.elems.end();
                     x++) {
                        stream << " ";
                        (*x)->exec(*this);
                }
        }
        stream << ")";
}

void Representor::visit(PrimitiveFunction &fn) {
        stream << "<primitive " << fn.get_name() << ">";
}

void Representor::visit(LambdaFunction &lambda) {
        // e.g <lambda x y>
        stream << "<lambda ";
        if (!lambda.params.empty()) {
                stream << lambda.params.front();
                for (auto it = ++lambda.params.begin();
                     it != lambda.params.end(); it++) {
                        stream << " ";
                        stream << *it;
                }
        }
        stream << ">";
}

// thos is probably how the representor class will get used
std::ostream &operator<<(std::ostream &os, SExp &sexp) {
        auto repr = Representor(os);
        sexp.exec(repr);
        return os;
}