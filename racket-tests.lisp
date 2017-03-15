#lang racket
;;This file includes brief examples demonstrating all of the core functinality of the language
(define my-eval
  (let ((ns (make-base-namespace)))
    (lambda (expr) (eval expr ns))))


(define tests 
	(list 
											         ;; builtin types
		'( displayln "Hello, world!")                                            ;; strings 
		'( displayln  3.14159)                                                           ;; numbers
		'( displayln  #t)                                                                       ;; booleans
		'( displayln  displayln)                                                        ;; atoms / primitive functions
		'( displayln (lambda (le) 
				 ((lambda (f) (f f)) 
				     (lambda (f) 
				     	(le (lambda (x) ((f f) x)))))))              ;;Lambda functions
				     	
		'( + 2 5)                                                                                        ;;Math primitives
		'(- 3 1)
		'(+ 3 -5)
		'(* 5 2) 
		'(/ 10 2) 
		'(modulo 101 10) 
		'( + 1 2 3 4 5 6 7 8 9 10)
		'(* 1 2 3 4 5 6 7 8 9 10)
		'(+ (* 2 10) (/ 100 5))
		
		'(car '(1 2 3 4 5))                                                                   ;; LIst primitives
		'(cdr '(1 #f 3 "bread" "butter"))
		'(cons "Killing's Equation" '()) 
		'(cons 5 (cons 4 (cons 3 (cons 2 ( cons 1 '()))))) 
		'(list "This" "is" "a" "list")
		
		'(if #t "this" "not this")                                                   ;; Logic, comparisons, and equality 
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
		
		'(displayln "The next few tests are quite long: they're much more nicely formatted in the source file!")
		
		'((lambda void                                                                  ;; lambdas and definitions (these are a bit long to read on the command line!) 
			(define x 20)
			(define y 20)
			(+ x y)) '()) 
		'((lambda void                                                                  ;; Demonstrates local variables 
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
			(displayln (g 2))) '())
		'((lambda void                                                                  ;; Demonstrates closure over local variables
			(define add-n 
				(lambda (n)
					(lambda (x) (+ x n)))) 
			(define add-5 (add-n 5))
			(add-5 5)) '())
		'(map (lambda (x) (* x x)) '(1 2 3 4 5 6  7 8 9 10))                       ;; Demonstrates maps, filters and folds
		'(filter (lambda (x) ( = (modulo x 2) 0) ) '(1 2 3 4 5 6 7 8 9 10))
		'(foldl (lambda (x acc) (cons x acc)) '() '(1 2 3 4 5 6))
			
	))

(displayln tests) 

(define execute-test 
	(lambda (test) 
		(display "Test:  ")
		(displayln test) 
		(display "Result:  ")
		(displayln (my-eval test)) 
		(displayln "") ))
		

(displayln "Starting tests...") 
(map execute-test tests)