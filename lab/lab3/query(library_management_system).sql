#table loan
SELECT * FROM loan;

#table employee
SELECT * FROM employee;

#table books
SELECT * FROM books;

#1
SELECT *
FROM employee
WHERE employee.name = 'Jones';

#2
SELECT employee.name
FROM employee
WHERE employee.empno=1 OR employee.empno=2;

#3
SELECT employee.name
FROM employee
WHERE employee.empno!=1;

#4
SELECT employee.name
FROM employee
WHERE employee.age>=25 AND employee.age<=30;

#5
SELECT *
FROM employee
WHERE employee.name LIKE 'J%';

#6
SELECT DISTINCT books.publisher
FROM books;

#7
SELECT books.isbn, books.title, books.authors, books.publisher
FROM books, loan
WHERE books.isbn=loan.isbn AND loan.loan_date>'2010-10-31';

#8
SELECT  employee.empno, COUNT(loan.empno)
FROM loan, employee
WHERE employee.empno=loan.empno
GROUP BY employee.empno
ORDER BY employee.empno ASC;