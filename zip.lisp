(define zip 
	(lambda (l1 l2) 
		(if (or (null? l1) (null? l2))
		'()
		(cons (list (car l1) (car l2)) (zip (cdr l1) (cdr l2))))))

(displayln (zip '(1 2 3 4 5 6 7 8) '(3 2 1 4 5 2 1)))
