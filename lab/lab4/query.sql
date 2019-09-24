#table loan
SELECT * FROM loan;

#table books
SELECT * FROM books;

#table employee
SELECT * FROM employee;

#start with... (4 options)
#---nothing after
SELECT *
FROM employee
WHERE employee.name = 'J';
#---exactly one letter behind
SELECT *
FROM employee
WHERE employee.name LIKE 'J_';
#---any letters after(0 ~ many)
SELECT *
FROM employee
WHERE employee.name LIKE 'J%';
#---the same with the last one
SELECT *
FROM employee
WHERE employee.name regexp '^J';

#1
SELECT books.isbn
FROM books
WHERE books.price > SOME(SELECT books.price
						 FROM books
						 WHERE books.publisher='Fudan');

#2
#---old version
SELECT COUNT(DISTINCT loan.isbn)
FROM loan
WHERE loan.isbn IN(SELECT loan.isbn
				   FROM loan
                   WHERE loan.loan_date LIKE '2018%'
                   GROUP BY loan.isbn);
#---better version
SELECT COUNT(DISTINCT loan.isbn)
FROM loan
WHERE loan.loan_date LIKE '2018%';
                   
#3
SELECT loan.isbn
FROM loan
WHERE loan.loan_date LIKE '2018%'
GROUP BY loan.isbn
HAVING COUNT(loan.isbn) = 1;

#4
SELECT books.publisher
FROM books
GROUP BY books.publisher
HAVING AVG(books.price) >= ALL(SELECT AVG(books.price)
							   FROM books
                               GROUP BY books.publisher);
                               
#5
INSERT INTO loan
	VALUES(1, (SELECT books.isbn
			   FROM books
               WHERE books.price >= ALL(SELECT books.price FROM books)), '2018-4-2');
SELECT * FROM loan;