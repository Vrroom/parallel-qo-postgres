# Parallel Query Optimizer for Postgres

We replaced the original query optimizer in Postgres with a parallel query optimizer based on [Parallelizing Query Optimization on Shared-Nothing Architectures](https://github.com/Vrroom/parallel-qo-postgres/blob/master/p660-trummer.pdf). Given a complex SQL query, containing multiple joins such as: 

```
SELECT * FROM table1, table2, table3, table4, table5 WHERE table1.id = table2.id AND table3.id = table4.id;
```

The algorithm finds the optimal plan for computing the final table. A plan can be thought of as a binary tree. Each node in the binary tree represents either a table in the query or a table constructed by joining the children of the node. The cost of a plan is the sum total of the costs of computing the join represented by each node of the binary tree. Different plans can result in vastly different query execution times. Hence, it is essential to find an efficient plan. Given join cost estimates, this problem is NP-Hard making a parallel query optimizer desirable.

## Installation Instructions

This repository is a fork of the original [Postgres repo](https://github.com/postgres/postgres). The build system is the same as theirs. However, we have an additional GLIB dependency due to which we had to change the installation instructions slightly. We suggest that you install this code into a local directory. In the instructions, we create a local directory `install` where the RDBMS is installed.

```
$ git clone https://github.com/Vrroom/parallel-qo-postgres.git
$ cd parallel-qo-postgres
$ export PKG_CONFIG=`which pkg-config`
$ mkdir install
$ ./configure --prefix=`pwd`/install --with-glib
$ make && make install
```

## Example

To test this work it out, you can load the toy [Pagila](https://github.com/devrimgunduz/pagila) database. I have provided a script to load the database. 

```
$ ./create_toy_database.sh
```

After this, change directory to the installation directory and run psql, supplying it with `test.sql`.

```
$ cd install/bin/
$ ./psql -d pagila -f ../../test.sql
```

For example, for the query:

```
EXPLAIN SELECT * 
FROM actor, film_actor, film, inventory
WHERE actor.actor_id = film_actor.actor_id AND
film_actor.film_id = film.film_id AND 
film.film_id = inventory.film_id;
```

We get the following plan:

```
 Hash Join  (cost=217.07..647.59 rows=25021 width=451)
   Hash Cond: (film_actor.film_id = inventory.film_id)
   ->  Hash Join  (cost=84.00..197.65 rows=5462 width=431)
         Hash Cond: (film_actor.film_id = film.film_id)
         ->  Hash Join  (cost=6.50..105.76 rows=5462 width=41)
               Hash Cond: (film_actor.actor_id = actor.actor_id)
               ->  Seq Scan on film_actor  (cost=0.00..84.62 rows=5462 width=16)
               ->  Hash  (cost=4.00..4.00 rows=200 width=25)
                     ->  Seq Scan on actor  (cost=0.00..4.00 rows=200 width=25)
         ->  Hash  (cost=65.00..65.00 rows=1000 width=390)
               ->  Seq Scan on film  (cost=0.00..65.00 rows=1000 width=390)
   ->  Hash  (cost=75.81..75.81 rows=4581 width=20)
         ->  Seq Scan on inventory  (cost=0.00..75.81 rows=4581 width=20)
```

If we check what the plan devised by the `standard_join_planner`, we get:

```
Hash Join  (cost=229.15..626.41 rows=25021 width=451)
   Hash Cond: (film_actor.film_id = film.film_id)
   ->  Hash Join  (cost=6.50..105.76 rows=5462 width=41)
         Hash Cond: (film_actor.actor_id = actor.actor_id)
         ->  Seq Scan on film_actor  (cost=0.00..84.62 rows=5462 width=16)
         ->  Hash  (cost=4.00..4.00 rows=200 width=25)
               ->  Seq Scan on actor  (cost=0.00..4.00 rows=200 width=25)
   ->  Hash  (cost=165.39..165.39 rows=4581 width=410)
         ->  Hash Join  (cost=77.50..165.39 rows=4581 width=410)
               Hash Cond: (inventory.film_id = film.film_id)
               ->  Seq Scan on inventory  (cost=0.00..75.81 rows=4581 width=20)
               ->  Hash  (cost=65.00..65.00 rows=1000 width=390)
                     ->  Seq Scan on film  (cost=0.00..65.00 rows=1000 width=390)
```

There are some slight differences in the plans for the two algorithms, but importantly, the total cost is similar. So that is good news 😋.  

## Algorithm

## Comparison with Postgres

## Known Bugs and Future Work

## References
