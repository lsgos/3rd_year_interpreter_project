
;;This file includes brief examples demonstrating all of the core functionality of the language

(define my-map
	(lambda (f xs)
		(if (null? xs)
		'()
		(cons (f (car xs)) (my-map f (cdr xs))))))

(define my-fold
	(lambda (f acc xs)
		(if (null? xs)
		acc
		(my-fold f (f acc (car xs)) (cdr xs)))))

(define tests
	(list

		;; builtin types
		;; strings
		'( displayln "Hello, world!")
		'( displayln "Hello, world\n These are the tests for my lisp interpreter")
		;; numbers
		'( displayln  3.14159)
		;; booleans
		'( displayln  #t)
		;; atoms / primitive functions
		'( displayln  displayln)

		;;Lambda functions
		'( displayln (lambda (le)
				 ((lambda (f) (f f))
				     (lambda (f)
				     	(le (lambda (x) ((f f) x)))))))


		;;Math primitives

		'( + 2 5)
		'(- 3 1)
		'(+ 3 -5)
		'(* 5 2)
		'(/ 10 2)
		'(% 101 10)
		'( + 1 2 3 4 5 6 7 8 9 10)
		'(* 1 2 3 4 5 6 7 8 9 10)
		'(+ (* 2 10) (/ 100 5))


		;; List primitives
		'(car '(1 2 3 4 5))
		'(cdr '(1 #f 3 "bread" "butter"))
		'(cons "Killing's Equation" '())
		'(cons 5 (cons 4 (cons 3 (cons 2 ( cons 1 '())))))
		'(list "This" "is" "a" "list")


		;; Logic, comparisons, and equality
		'(if #t "this" "not this")
		'(if #f "not this" "but this")
		'(if 39 "Everything is true" "but false")
		'(if (not (lambda x (* x 2))) 99 "this is still true!")
		'(= 4 5)
		'(= 4 (* 2 2))
		'(eq? 4 5)
		'(eq? 4 "4")
		'(eq? "this" "this")
		'(eq? #t #f)
		'(null? 4)
		'(null? "null")
		'(null? null)
		'(null? '())
		'(null? (cdr '(2)))
		'(number? 3.14159)
		'(number? "3.14159")
		'(and #t #f)
		'(and #t #t)
		'(or #t #f)
		'(or #f #f)
		'(and #t #f #f #f #t #f)
		'(and #t #t #t #t #t #t)
		'(or #f #f #f #f #f)
		'(or #f #f #f #f #t)

		'(displayln "The next few tests are quite long: they're much more nicely formatted in the source file!")

		;; lambdas and definitions (these are a bit long to read on the command line!)

		'((lambda ()
			(define x 20)
			(define y 20)
			(+ x y)) 
			)

		;; Demonstrates local variables

		'((lambda ()
			(define a-local-var 7)
			(define f
				(lambda (x)
					(define  a-local-var 5)
					(* x  a-local-var)))
			(define g
				(lambda (x)
					(define  a-local-var 6)
					(* x  a-local-var)))
			(displayln "")
			(displayln (f 2))
			(displayln (g 2))
			(displayln a-local-var) ;;this demonstrates the global version of a-local-var has not been affected 
			)
		)
		;; Demonstrates closure over local variables

		'((lambda ()
			(define add-n
				(lambda (n)
					(lambda (x) (+ x n))))
			(define add-5 (add-n 5))
			(add-5 5))
		)

		;; Demonstrates maps, filters and folds

		'(map (lambda (x) (* x x)) '(1 2 3 4 5 6  7 8 9 10))
		'(filter (lambda (x) ( = (% x 2) 0) ) '(1 2 3 4 5 6 7 8 9 10))
		'(fold (lambda (acc x) (cons x acc)) '() '(1 2 3 4 5 6))

		;;just for fun: this is well known quine, a program whose output is it's own source code

		'((lambda (s) (displayln (list s (list (quote quote) s))))
			(quote (lambda (s) (displayln (list s (list (quote quote) s))))))

		;TODO add files to the test 
		'((lambda () 
				(define f (open-output-port "Hello.txt"))
				(if f 
				(displayln "Hello, world!" f)
				(displayln "Could not open file")
				)
				(close-output-port f)
		 ))
	))


(define execute-test
	(lambda (test)
		(display "Input:   ")
		(displayln test)
		(display "Result:  ")
		(displayln (eval test))
		(displayln "") ))


(displayln "Starting tests...")
(map execute-test tests)