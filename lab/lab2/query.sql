#select/display the whole table
SELECT * # * means all columns
FROM dvd_collection.movies; #the source table to select in
#no condition here

#insert a movie
#you can only do this one time as this operation will insert the movie in the table permanently
INSERT INTO dvd_collection.movies #omit the tuple, as all columns are NN(not null)
	VALUES('4','The Shape of Water','2018-03-15','6');

#select/display the whole table
SELECT * 
FROM dvd_collection.movies;

#select the movie_title with rating less than 5 and more than 3
SELECT movies.movie_title #select columns that are wanted
FROM dvd_collection.movies #the source table to select in
WHERE movies.rating>3 AND movies.rating<5; #conditions required in this select

#select/display the whole table
SELECT * 
FROM dvd_collection.movies; 

#select
SELECT movies.movie_id, movies.movie_title, movies.rating #select columns that are wanted, and display in this order
FROM dvd_collection.movies #the source table to select in
WHERE movies.rating>4 OR movies.rating>5; #conditions required in this select

#select/display the whole table
SELECT * 
FROM dvd_collection.movies; 

#update a release date
UPDATE dvd_collection.movies
SET movies.release_date = '2018-03-16'
WHERE movies.movie_id = 4;

#select/display the whole table
SELECT * 
FROM dvd_collection.movies;

#delete the movie 4
DELETE FROM dvd_collection.movies
WHERE movies.movie_id = 4;

#select/display the whole table
SELECT * 
FROM dvd_collection.movies;