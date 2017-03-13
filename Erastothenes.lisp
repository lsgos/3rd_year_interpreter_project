;;This program uses my interpreted language to find all the prime numbers lower than 1000, 
;;using the Sieve of Erastothenes. 

;;This function shows an example of using the builtin filter function to do the 'heavy lifting' of a for loop

(define range 
	(lambda (i n) 
		(if (= i n)
			'()
			(cons i (range (+ i 1) n)))))

(define rm-multiples-of 
	(lambda (arg xs)
		(filter  (lambda x (not (= (% x arg) 0))) xs)))
			
(define erastothenes 
	(lambda (xs)
		(if (null? xs) 
		'() 
		(cons (car xs) (rm-multiples-of (car xs) (erastothenes (cdr xs)))))))
		
(displayln (erastothenes (range 2 1000)))