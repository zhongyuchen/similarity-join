use book;

select distinct X.isbn
from books as X, books as Y
where X.price > Y.price and Y.publisher = 'Fudan';

select count(distinct isbn)
from loan
where loan_date > '2018';

select isbn
from loan
where loan_date > '2018'
group by isbn
having count(isbn) = 1;

select publisher
from books
group by publisher
having avg(price) >= all (
	select avg(price) 
	from books 
	group by publisher);

insert into loan values 
(1, (select isbn from books where price >= all (select price from books)), '2018-4-2');