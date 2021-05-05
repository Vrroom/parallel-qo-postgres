# (not) Parallel Query Optimizer for Postgres

We replaced the original query optimizer in Postgres with a (not) parallel query optimizer based on [Parallelizing Query Optimization on Shared-Nothing Architectures](https://github.com/Vrroom/parallel-qo-postgres/blob/master/p660-trummer.pdf). Given a complex SQL query, containing multiple joins such as: 

```
SELECT * FROM table1, table2, table3, table4, table5 WHERE table1.id = table2.id AND table3.id = table4.id;
```

The algorithm finds the optimal plan for computing the final table. A plan can be thought of as a binary tree. Each node in the binary tree represents either a table in the query or a table constructed by joining the children of the node. The cost of a plan is the sum total of the costs of computing the join represented by each node of the binary tree. Different plans can result in vastly different query execution times. Hence, it is essential to find an efficient plan. Given join cost estimates, this problem is NP-Hard making a (not) parallel query optimizer desirable.

## Why is this called _(not) parallel_

Because, I wasn't able to parallelize the worker processes in Postgres 😓. Pseudocode of current main routine:

```python3
parallel_join_search (query_data, n_workers) :
  worker_plans = []
  for worker_id in range(n_workers) :
    # Worker partitions planning space based on worker_id
    worker_plans.append(worker(query_data, worker_id))
  costs = [plan.cost for plan in worker_plans]
  # Return minimum cost plan
  return worker_plans[argmin(costs)]
```

Replacing the `for` loop with a `parfor` of is tricky in Postgres. Here are some attempts that I made to do this:

1. _Using Pthreads_: A Postgres backend process assumes that it contains a single stack. It is not thread safe. Hence using `pthreads` invariably leads to segmentation faults which are not easily traceable.
2. _Using Postgres Internal Parallel Library_: In order to explain this attempt, you need to know some details of how Postgres handles queries. Normally, you launch a server.
```
$ ./postgres -D data/
```
and connect to it using a client such as `psql`. The server is called `postmaster`. `postmaster` handles each query written in `psql` by forking a child process, let's call it  process `B`, that executes the query. `B` is responsible for query planning, of which, query optimization is a part. `postmaster` and `B` communicate via shared memory. Postgres' Internal Parallel Library `src/include/access/parallel.h` is built on top of this architecture. The `B` can use this library as follows:
  * Set up a `ParallelContext` with the entry point for each worker.
  * Set up shared memory. Fill the shared memory with data that the workers need to accomplish their task.
  * Pass the `ParallelContext` and the shared memory segment to parent process - `postmaster`.
`postmaster` then starts these worker processes (via fork). They read the shared memory to get relevant parameters. Since the fork is initiated by the `postmaster`, workers don't have a duplicate of the virtual memory of `B`. All the data needed by the worker needs to be serialized and stored in shared memory by `B`. We need to serialize `query_data`, from the pseudocode above. Doing this is not easy! 
3. _Using fork_: Finally I tried to fork directly from `B`. Each worker gets a copy of `B`'s virtual memory. Each worker also gets a copy of `B`'s shared memory. When they forage through the shared memory without synchronization, all kinds of bad things happen. I'm not aware of how to do this synchronization correctly. 

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

Although there are differences in the two plans, the total cost estimate are almost the same. This is good news 😋.

## Algorithm

We find the optimal left deep join plan. Consider joining relations a, b, c and d. A left deep join plan is of the form (((a ⋈ b) ⋈ c) ⋈ d). To simplify matters, a left deep join plan is a string a ⋈ b ⋈ c ⋈ d where join operations are executed in order, from left to right. Simplest way of finding the optimal plan is to enumerate all permutations and estimate their costs. We can be slightly smart and use dynamic programming to build the optimal order from bottom-up. In the DP table, we store the optimal join order for all subsets of relations. Then, we can find the optimal join order for the larger subset as follows:

```python
for i, right in enumerate(rels) : 
  left = rels - {i} # Seperate out ith rel from the set
  tryJoin = join(DP[left], right) # ((optimal join order for smaller set) ⋈ i)
  cost = estimateJoinCost(tryJoin)
  if DP[rels].cost > cost : # Check whether we have found a better join order
    DP[rels] = tryJoin 
```

The corresponding code can be found [here](https://github.com/Vrroom/parallel-qo-postgres/blob/30846f6cbd9234bb480794f8c850b454ace05d6d/src/backend/optimizer/parallel/parallel_worker.c#L193).

In order to split the dynamic programming computation across many workers, we place constraints on the join orders that each worker searches through. First, the relations are grouped in pairs.

(a, b), (c, d)

On each pair, a worker places constraints on pairs of the form: _relation 1 will be before relation 2 in the join order_. So if we have 4 workers, we'll split the space of join orders as: 

1. Worker 1: a before b and c before d.
2. Worker 2: b before a and c before d.
3. Worker 3: a before b and d before c.
4. Worker 4: b before a and d before c.

For example, Worker 1 will search over the permutations:

1. a, b, c, d
2. a, c, b, d
3. a, c, d, b
4. c, a, d, b
5. c, d, a, b
6. c, a, b, d

The dynamic programming step is suitably modified so that each worker only searches for valid orders under its constraints. The remaining crucial bit is the `estimateJoinCost` step in the pseudo-code. This is done in [src/backend/optimizer/parallel/parallel_eval.c](https://github.com/Vrroom/parallel-qo-postgres/blob/30846f6cbd9234bb480794f8c850b454ace05d6d/src/backend/optimizer/parallel/parallel_eval.c#L40). Given a plan, we use Postgres' existing planning machinery to combine the relations in the order specified. Given an order, Postgres evaluates the sizes of the tables, effect of join clauses (is the data sorted by the variable in the join clause in which case MERGE JOIN may be fast), efficiency of scanning tables (do I use sequential scan or is the data indexed?) among other things. In some cases, due to semantic restrictions imposed by the SQL query, the join order may not even be feasible. 

## Limitations

1. Not Parallel! If anyone knows how to do this. Please feel free to reach out to me. 
2. Doesn't evaluate bushy plans or arbitrary binary trees. For example: (a ⋈ (b ⋈ d) ⋈ c).

## Acknowledgements

This work started as a course project for a database course (CS 317) at IITB under [S. Sudarshan](https://www.cse.iitb.ac.in/~sudarsha/). My team-mates were [Adwait](https://github.com/adwait), [Nitish](https://github.com/joshinh), Nilay. This work profited immensely from discussions with [Julien](https://github.com/rjuju). 


