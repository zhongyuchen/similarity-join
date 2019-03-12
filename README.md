# similarity-join

![build status](https://img.shields.io/badge/build-passing-66c2a5.svg)
[![psql version](https://img.shields.io/badge/PostgreSQL-10.4-fc8d62.svg)](https://www.postgresql.org/)
[![Apache License](https://img.shields.io/badge/license-Apache2.0-8da0cb.svg)](http://www.apache.org/licenses/)

Extend PostgreSQL with similarity join feature!

## Similarity Join

There are 2 versions of this similarity join feature, which are implemented based on 2 different methods for quantifying
 similarity between 2 strings:
* Levenshtein distance
* Jaccard index

## Manual

### How to Install PostgreSQL

Run the following commands at the root directory of the PostgreSQL package:
```commandline
./configure
make
su
make install
adduser postgres
mkdir /usr/local/pgsql/data
chown postgres /usr/local/pgsql/data
su - postgres
/usr/local/pgsql/bin/initdb -D /usr/local/pgsql/data
/usr/local/pgsql/bin/postgres -D /usr/local/pgsql/data >logfile 2>&1 &
```

### How to Re-install PostgreSQL

* If you want to test the performance of different versions of this extension, you'll need to re-install PostgreSQL 
before running another version
* Run the following commands at the root directory of the PostgreSQL package:
```commandline
make clean
sudo rm -r /usr/local/pgsql/data
make
su
make install
adduser postgres
mkdir /usr/local/pgsql/data
chown postgres /usr/local/pgsql/data
su - postgres
/usr/local/pgsql/bin/initdb -D /usr/local/pgsql/data
/usr/local/pgsql/bin/postgres -D /usr/local/pgsql/data >logfile 2>&1 &
```

### How to Load Dataset

* There is a dataset provided, which is `/handout/similarity_data.sql`
* You can load the dataset by the following command, replacing `<PATH>` with the exact path of the dataset:
```commandline
/usr/local/pgsql/bin/psql -f <PATH>
```

### How to Enter PostgreSQL

```commandline
/usr/local/pgsql/bin/psql
```

### How to Enable Timing

Run the following command after you've enter PostgreSQL
```commandline
\timing
```

### How to Run Similarity Join

* Run similarity join based on Levenshtein distance:
```sql
select count(*) from restaurantphone rp, addressphone ap where
levenshtein_distance(rp.phone, ap.phone) < 4;
select count(*) from restaurantaddress ra, restaurantphone rp where
levenshtein_distance(ra.name, rp.name) < 3;
select count(*) from restaurantaddress ra, addressphone ap where
levenshtein_distance(ra.address, ap.address) < 4;
```

* The other version, which is also based on Levenshtein distance, also takes in the distance:
```sql
select count(*) from restaurantphone rp, addressphone ap where
levenshtein_distance_2(rp.phone, ap.phone, 4);
select count(*) from restaurantaddress ra, restaurantphone rp where
levenshtein_distance_2(ra.name, rp.name, 3);
select count(*) from restaurantaddress ra, addressphone ap where
levenshtein_distance_2(ra.address, ap.address, 4);
```

* Run similarity join based on Jaccard index:
```sql
select count(*) from restaurantphone rp, addressphone ap where
jaccard_index(rp.phone, ap.phone) > 0.6;
select count(*) from restaurantaddress ra, restaurantphone rp where
jaccard_index(ra.name, rp.name) > 0.65;
select count(*) from restaurantaddress ra, addressphone ap where
jaccard_index(ra.address, ap.address) > 0.8; 
```

### Results

* By Levenshtein distance: 3252, 2130, 2592
* By Jaccard index: 1647, 2347, 2120

### How to Exit PostgreSQL

```commandline
\q
```

## Similarity Join Extension

### Extending by C Functions

#### Compile

* Run the following commands at the directory containing `similarity_join.c` file, 
replacing `<PATH>` with the exact path of `/postgresql-10.4/src/include`
```commandline
cc -I <PATH> -fPIC -c similarity_join.c
cc -shared -o similarity_join.so similarity_join.o
```

#### Create Functions

* Create similarity join function based on Levenshtein distance, 
replacing `<PATH>` with the exact path of `/postgresql-10.4/src/tutorial/similarity_join`
```sql
CREATE FUNCTION levenshtein_distance(text, text) RETURNS integer 
	AS '<PATH>', 
	   'levenshtein_distance' 
	LANGUAGE C STRICT;
```

* Drop this function by:
```sql
DROP FUNCTION levenshtein_distance(text, text);
```

* The other version of Levenshtein distance:
```sql
CREATE FUNCTION levenshtein_distance_2(text, text, integer) RETURNS boolean
	AS '<PATH>', 
	   'levenshtein_distance_2' 
	LANGUAGE C STRICT;
	
DROP FUNCTION levenshtein_distance_2(text, text, integer);
```

* The function based on Jaccard index:
```sql
CREATE FUNCTION jaccard_index(text, text) RETURNS double precision 
	AS '<PATH>', 
	   'jaccard_index' 
	LANGUAGE C STRICT;

DROP FUNCTION jaccard_index(text, text);
```

#### Run

Run similarity join as mentioned before.

### Extending by Core Modification

#### Install

* You can replace the following files in the PostgreSQL package with 
the files in `\extending-by-core-modification\modified-files` before installation:
`\postgresql-10.4\src\backend\utils\adt\oracle_compat.c`
`\postgresql-10.4\src\backend\utils\fmgroids.h`
`\postgresql-10.4\src\backend\utils\fmgrprotos.h`
`\postgresql-10.4\src\backend\utils\fmgrtab.c`
`\postgresql-10.4\src\include\catalog\pg_proc.h`

* You can simply install the modified PostgreSQL package in `\extending-by-core-modification\modified-package`

#### Run

After installation, you can run similarity join as mentioned before.

## Results

In general:
* Modifying the core brings much better performance than creating C functions.
* The Levenshtein distance function that takes in the distance is much faster than the one that doesn't take in the distance.

## Links

* Visit [PostgreSQL](https://www.postgresql.org/download/) to download the original package
