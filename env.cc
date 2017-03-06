
#include "env.h"
#include "sexp.h"
// for now, just implement numeric primitives

// TODO try to come up with a way to abstract the creation of these things? a
// lot of boilerplate here
// Maybe use a different helper function for special forms and functions?

// Function to abstract boilerplate in builtin function generation

// a function to abstract the creation of numeric primitives functions
SExp *GlobalEnv::mk_numeric_primitive(
    std::function<double(double acc, double x)> func, std::string funcname) {

        auto const fn = [func, funcname](std::list<SExp *> args,
                                         Env &env) -> SExp * {
                // check the list is not empty
                if (args.empty()) {
                        throw evaluation_error(
                            "Incorrect number of arguments in function " +
                            funcname);
                }
                for (auto it = args.begin(); it != args.end(); ++it) {
                        (*it) = (*it)->eval(env);
                }
                double acc;
                for (auto it = args.begin(); it != args.end(); ++it) {
                        if ((*it)->type() != LispType::Number) {
                                throw evaluation_error("Non numeric arguments "
                                                       "encountered in "
                                                       "function " +
                                                       funcname);
                        }
                        // now we know its a number, we are free to down-cast
                        // the pointer to
                        // access it's numeric field. We need to use get() to
                        // access the raw
                        // pointer managed by the unique pointer wrapper
                        double num = (static_cast<Number *>(*it))->val();

                        if (it == args.begin()) {
                                acc = num;
                        } else {
                                acc = func(acc, num);
                        }
                }
                SExp *result = env.allocate(new Number(acc));
                return result;
        };
        return heap.allocate(new PrimitiveFunction(fn, funcname));
}

// called to create a blank environment: bind the language builtins
GlobalEnv::GlobalEnv() { bind_primitives(); }

// this creates a new closure, pointing to the one that created it.
SExp *Env::lookup(std::string id) {

        auto x = scope.find(id);
        if (x != scope.end())
                return x->second;

        if (global != nullptr) {
                // if there is a link-back to a global scope
                return global->lookup(id);
        }
        return nullptr;
}

void GlobalEnv::bind_primitives() {
        def("+",
            mk_numeric_primitive(
                [](double acc, double x) -> double { return acc + x; }, "+"));
        def("-",
            mk_numeric_primitive(
                [](double acc, double x) -> double { return acc - x; }, "-"));
        def("*",
            mk_numeric_primitive(
                [](double acc, double x) -> double { return acc * x; }, "*"));
        def("/",
            mk_numeric_primitive(
                [](double acc, double x) -> double { return acc / x; }, "/"));
        def("cons", mk_cons());
        def("car", mk_car());
        def("quote", mk_quote());
        def("define", mk_define());
        def("lambda", mk_lambda());
        def("cdr", mk_cdr());
        def("if", mk_if());
        def("null?", mk_isnull());
        def("exit", mk_exit());
        def("=", mk_numeric_equals());
        def("eq?", mk_eq());
        return;
}

GlobalEnv::~GlobalEnv() {}

Env GlobalEnv::capture_scope() {
        // create a new Env, closing over the current scope.
        return Env(*this);
}

Env::Env(GlobalEnv &g) : global(&g) { scope = g.scope; }

// defineing is only legal in the global scope
void Env::def(std::string id, SExp *value) {
        // bind to the global scope
        scope[id] = value;
        // auto repr = Representor(std::cout);
        // std::cout << "Defining value " << id << " as ";
        // value->exec(repr);
        // std::cout << std::endl;
        return;
}

SExp *Env::allocate(SExp *new_obj) {
        // keep all control of allocation in the global scope
        return global->allocate(new_obj);
}
SExp *GlobalEnv::mk_cons() {
        auto cons = [](std::list<SExp *> args, Env &env) -> SExp * {
                if (args.size() != 2) {
                        throw evaluation_error(
                            "Incorrect number of arguments in primitive cons");
                }
                for (auto it = args.begin(); it != args.end(); ++it) {
                        (*it) = (*it)->eval(env);
                }
                SExp *car = args.front();
                args.pop_front();
                SExp *cdr = args.front();
                // technically this is different from classical lisp as we don't
                // have a
                // dotted-list type (which I don't see as particularly useful)
                if (cdr->type() != LispType::List) {
                        throw evaluation_error("Cannot cons onto a non-list "
                                               "type [expected: (cons T "
                                               "list)]");
                }
                // push the car argument onto the list of the second, then
                // return the
                // second
                // argument.
                // Copy the list: this only copies the pointers, so it should be
                // relatively
                // cheap
                auto list_cdr = static_cast<List *>(cdr);
                std::list<SExp *> cons_list = list_cdr->elems;
                cons_list.push_front(car);
                return env.allocate(new List(cons_list));
        };
        return heap.allocate(new PrimitiveFunction(cons, "cons"));
}

SExp *GlobalEnv::mk_car() {
        auto car = [](std::list<SExp *> args, Env &env) -> SExp * {
                if (args.size() != 1) {
                        throw evaluation_error(
                            "Incorrect number of arguments in primitive car");
                }
                SExp *ls = args.front();
                ls = ls->eval(env);
                if (ls->type() != LispType::List) {
                        throw evaluation_error(
                            "Cannot ask for the car of a non-list");
                }
                auto elems = static_cast<List *>(ls)->elems;
                if (elems.empty()) {
                        throw evaluation_error(
                            "Cannot ask for the car of an empty list");
                }
                auto car = elems.front();
                return car;
        };
        return heap.allocate(new PrimitiveFunction(car, "car"));
}
SExp *GlobalEnv::mk_isnull() {
        auto isnull = [](std::list<SExp *> args, Env &env) {
                bool result = false;
                if (args.size() != 1) {
                        throw evaluation_error(
                            "Incorrect number of arguments in functino null?");
                } else {
                        SExp *obj = args.front()->eval(env);
                        if (obj->type() == LispType::List &&
                            static_cast<List *>(obj)->elems.empty()) {
                                result = true;
                        }
                        return env.allocate(new Bool(result));
                }
        };
        SExp *primitive_isnull =
            heap.allocate(new PrimitiveFunction(isnull, "null?"));
        return primitive_isnull;
}

SExp *GlobalEnv::mk_cdr() {
        auto cdr = [](std::list<SExp *> args, Env &env) {
                if (args.size() != 1) {
                        throw evaluation_error(
                            "Incorrect number of arguments in primitive cdr");
                }
                SExp *ls = args.front();
                ls = ls->eval(env);
                if (ls->type() != LispType::List) {
                        throw evaluation_error(
                            "Cannot ask for the cdr of a non list type");
                }
                auto elems =
                    static_cast<List *>(ls)->elems; // copy list of object
                if (elems.empty()) {
                        throw evaluation_error(
                            "Cannot ask for the cdr of a empty list");
                }
                elems.pop_front();
                return env.allocate(new List(elems));
        };
        SExp *primitive_cdr = heap.allocate(new PrimitiveFunction(cdr, "cdr"));
        return primitive_cdr;
}

SExp *GlobalEnv::mk_quote() {
        auto quote = [](std::list<SExp *> args, Env &env) -> SExp * {
                if (args.size() != 1) {
                        throw evaluation_error(
                            "Incorrect number of arguments in primitive quote");
                }
                SExp *atom = args.front();
                return atom;
        };
        SExp *primitive_quote =
            heap.allocate(new PrimitiveFunction(quote, "quote"));
        return primitive_quote;
}

SExp *GlobalEnv::mk_define() {
        auto define = [](std::list<SExp *> args, Env &env) -> SExp * {
                if (args.size() != 2) {
                        throw evaluation_error(
                            "Incorrect number of arguments in primitive "
                            "define: expected two");
                }
                auto atom = args.front();
                args.pop_front();
                if (atom->type() != LispType::Atom) {
                        throw evaluation_error("Expected atomic symbol as "
                                               "first argument to define");
                }
                std::string id = static_cast<Atom *>(atom)->get_identifier();
                auto value = args.front();
                value = value->eval(env);
                env.def(id, value);
                return env.allocate(new List);
        };
        SExp *primitive_define =
            heap.allocate(new PrimitiveFunction(define, "define"));
        return primitive_define;
}

SExp *GlobalEnv::mk_lambda() {
        auto lambda = [](std::list<SExp *> args, Env &env) -> SExp * {
                // lambda is a special form: evaluate the first argument, but
                // none of
                // the
                // others.
                if (args.size() < 2) {
                        throw evaluation_error(
                            "Too few arguments in call to lambda");
                }
                // first argument to lambda must be either a list of atoms or a
                // single
                // atom
                SExp *first = args.front();
                args.pop_front();
                LispType param_type = first->type();
                // this will be used to construct the lambda
                std::list<std::string> param_list;
                if (param_type == LispType::Atom) {
                        auto at = static_cast<Atom *>(first);
                        param_list.push_back(at->get_identifier());
                } else if (param_type == LispType::List) {
                        // check f it's a list of atoms
                        auto list = static_cast<List *>(first);
                        for (auto it = list->elems.begin();
                             it != list->elems.end(); ++it) {
                                if ((*it)->type() == LispType::Atom) {
                                        auto at = static_cast<Atom *>(*it);
                                        param_list.push_back(
                                            at->get_identifier());
                                } else {
                                        throw evaluation_error(
                                            "Error in arguments to lambda: "
                                            "excepted "
                                            "identifier or list of "
                                            "identifiers");
                                }
                        }
                } else {
                        // not valid:
                        throw evaluation_error(
                            "Error in first argument to lambda: expected "
                            "identifier or list of identifiers");
                }
                // capture scope
                Env closure = env.capture_scope();
                return env.allocate(
                    new LambdaFunction(closure, param_list, args));
        };
        SExp *primitive_lambda =
            heap.allocate(new PrimitiveFunction(lambda, "lambda"));
        return primitive_lambda;
}
SExp *GlobalEnv::mk_if() {
        // implement the if special form
        auto if_sf = [](std::list<SExp *> args, Env &env) -> SExp * {
                if (args.size() != 3) {
                        throw evaluation_error(
                            "Incorrect number of arguments in if special form");
                }
                auto predicate = args.front();
                args.pop_front();
                auto then_clause = args.front();
                args.pop_front();
                auto else_clause = args.front();
                // evaluate the predicate expressions
                predicate = predicate->eval(env);
                if (is_true(predicate)) {
                        return then_clause->eval(env);
                } else {
                        return else_clause->eval(env);
                }
        };
        SExp *primitive_if = heap.allocate(new PrimitiveFunction(if_sf, "if"));
        return primitive_if;
}

SExp *GlobalEnv::mk_exit() {
        auto exit = [](std::list<SExp *> args, Env &env) -> SExp * {
                throw exit_interpreter();
                return nullptr;
        };
        return heap.allocate(new PrimitiveFunction(exit, "exit"));
}

SExp *GlobalEnv::mk_numeric_equals() {
        auto eq = [](std::list<SExp *> args, Env &env) -> SExp * {
                if (args.size() < 2) {
                        throw evaluation_error("Too few arguments in primitive "
                                               "=; expected two or more");
                }
                bool result = true;
                SExp *first = args.front()->eval(env);
                if (first->type() != LispType::Number) {
                        throw evaluation_error(
                            "Found non numeric arguments in function =");
                }
                double last = static_cast<Number *>(first)->val();
                for (auto it = args.begin()++; it != args.end(); ++it) {
                        (*it) = (*it)->eval(env);
                        if ((*it)->type() != LispType::Number) {
                                throw evaluation_error("Found non numeric "
                                                       "arguments in function "
                                                       "=");
                        }
                        if (last != static_cast<Number *>(*it)->val()) {
                                result = false;
                                break;
                        }
                }
                return env.allocate(new Bool(result));
        };
        return heap.allocate(new PrimitiveFunction(eq, "="));
}

SExp *GlobalEnv::mk_eq() {
        auto eq = [](std::list<SExp *> args, Env &env) -> SExp * {
                // This is slightly different from the canonical lisp eq, which
                // compares for
                // pointer equality. This is in fact equivalent to schemes eqv.
                // However in my
                // implementation comparing for pointer equality is more or less
                // pointless as very
                // few objects will have equal pointers, so we may as well just
                // have the obvious
                // test for equality on primitives.

                if (args.size() != 2) {
                        throw evaluation_error("Incorrect number of arguments "
                                               "to eq?: expected two");
                }
                // eval args
                for (auto it = args.begin(); it != args.end(); ++it) {
                        (*it) = (*it)->eval(env);
                }

                bool result;

                auto arg1 = args.front();
                args.pop_front();
                auto arg2 = args.front();
                auto type1 = arg1->type();
                auto type2 = arg2->type();
                if (type1 != type2) {
                        result = false;
                }
                switch (type1) {
                case LispType::Number:
                        result = (static_cast<Number *>(arg1)->val() ==
                                  static_cast<Number *>(arg2)->val());
                        break;
                case LispType::String:
                        result = (static_cast<String *>(arg1)->val() ==
                                  static_cast<String *>(arg2)->val());
                        break;
                case LispType::Bool:
                        result = (static_cast<Bool *>(arg1)->val() ==
                                  static_cast<Bool *>(arg2)->val());
                        break;
                case LispType::PrimitiveFunction:
                        result = (arg1 == arg2);
                        break;
                default:
                        // all other types are compound: these are covered by
                        // equal?, so just compare pointer equality
                        result = (arg1 == arg2);
                }
                return env.allocate(new Bool(result));
        };
        return heap.allocate(new PrimitiveFunction(eq, "eq"));
}

SExp *Heap::allocate(SExp *new_object) {
        // objects.push_back(Cell(new_object));
        std::pair<SExp *, bool> entry(new_object, false);
        // std::cout << "Storing " << new_object << std::endl;
        objects.insert(entry);
        return new_object;
}

// The heap class is responsible for manageing the memory usage of
// the program, so it needs to clean up the pointers.
Heap::~Heap() {
        for (auto it = objects.begin(); it != objects.end(); ++it) {
                // std::cout << "Freeing " << it->first << std::endl;
                delete it->first;
        }
}

// setup the heap for mark and sweep: set all the usage marks to zero
void Heap::reset_marks() {
        for (auto it = objects.begin(); it != objects.end(); ++it) {
                it->second = false;
        }
}
// sweep memory, cleaning up anything marked for deletion.
void Heap::sweep() {
        for (auto it = objects.begin(), next = objects.begin();
             it != objects.end(); it = next) {
                next = it;
                ++next;

                if (!(it->second)) {
                        SExp *ptr =
                            it->first; // cleanup memory managed by this key
                        objects.erase(it);
                        // std::cout << "Freeing " << ptr << std::endl;
                        delete ptr; // remove from the heap
                }
        }
}

void Heap::mark(SExp *addr) {
        auto entry = objects.find(addr);
        if (entry == objects.end()) {
                throw implementation_error(
                    "This shouldn't happen: encountered unmanaged address");
        }
        // mark memory as used
        entry->second = true;
        // std::cout << "Marked " << entry->first << " as in use" << std::endl;
        auto expr_type = entry->first->type();
        // Lists and functions can contain references to other objects: we need
        // to
        // recursively trace out their tree.
        // can we implement this as a visitor? might be slightly neater. Or is
        // it
        // unnessarily complicated?
        if (expr_type == LispType::List) {
                auto list = static_cast<List *>(addr);
                for (auto it = list->elems.begin(); it != list->elems.end();
                     ++it) {
                        // mark all sub elements as well
                        mark(*it);
                }
        }
        if (expr_type == LispType::LambdaFunction) {
                auto lambda = static_cast<LambdaFunction *>(addr);
                // mark the expressions in the function body
                for (auto obj = lambda->body.begin(); obj != lambda->body.end();
                     ++obj) {
                        mark(*obj);
                }
                // iterate through the closure's bindings, marking the scope
                // captured
                // from
                // it's creation
                for (auto obj = lambda->closure.scope.begin();
                     obj != lambda->closure.scope.end(); ++obj) {
                        mark(obj->second);
                }
        }
        return;
}

// naive mark-and-sweep: take the environment bindings, and walk through the
// objects described by their bindings, marking everything reachable from the
// root nodes as in use. Then iterate through the object map freeing everything
// that wasn't reachable; these things have no bindings to the rest of the
// program
// and are thus garbage.
void Heap::collect_garbage(Env &env) {
        reset_marks();
        for (auto entry = env.scope.begin(); entry != env.scope.end();
             ++entry) {
                mark(entry->second);
        }
        sweep();
}