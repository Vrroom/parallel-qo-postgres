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

## Examples

Running examples is not straightforward 🙈. Our changes broke Postgres and we can handle only a subset of SQL queries. For example, we can't handle nested queries. I think the Postgres client will simply segfault if you try to run such queries. Anyway, to test it out, you will have to go back to the last official commit (`4a2994f055`) in this repo, build it, load the toy [Pagila](https://github.com/devrimgunduz/pagila) dataset, checkout master and build again! It is a lot of work, I'm sorry. I'll try to find a better way to integrate our implementation without breaking Postgres. Till then, follow the instructions below.

```
$ git checkout 4a2994f055
$ ./configure --prefix=`pwd`/install
$ make && make install
```

After this, change directory to installation directory and create the toy database.

```
$ cd install/bin
$ ./initdb -D data
$ ./postgres -D data/ &
$ ./createdb pagila
$ ./psql -d pagila -f ../../pagila/pagila-schema.sql
$ ./psql -d pagila -f ../../pagila/pagila-data.sql
$ ./psql -d pagila -f ../../pagila/pagila-insert-data.sql
```

Having created the dataset, checkout `master` and follow the installation instructions above. If anyone actually tries all this, I'm very sorry for all this extra effort 🙈.

## Algorithm

## Comparison with Postgres

## Known Bugs and Future Work

## References
