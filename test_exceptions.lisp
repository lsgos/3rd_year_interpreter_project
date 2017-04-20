;;Test the exception reporting utility in the interpreter: it ought to report
;;the exception that occured and the line where it happened.
;;It's more accurate for parser errors: for logic errors it reports the location of the end of
;;the expression that crashed the interpreter.

(define square (lambda (x) (* x x)))

(define a 15)
(define b 21)

(displayln ( + (square a) (square b)))

;;(displayln (+ (cube a) (cube b))) ;;ERROR HERE; cube undefined

(displayln "Hello, world!")) ;;Error here, too many closing parens