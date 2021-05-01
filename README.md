# Parallel Query Optimizer for Postgres

We replaced the original query optimizer in Postgres with a parallel query optimizer based on Parallelizing Query Optimization on Shared-Nothing Architectures. Given a complex SQL query, containing multiple joins such as: 

```
SELECT * FROM table1, table2, table3, table4, table5 WHERE table1.id = table2.id AND table3.id = table4.id;
```

The algorithm finds the optimal plan for computing the final table. A plan can be thought of as a binary tree. Each node in the binary tree represents either a table in the query or a table constructed by joining the children of the node. The cost of a plan is the sum total of the costs of computing the join represented by each node of the binary tree. Different plans can result in vastly different query execution times. Hence, it is essential to find an efficient plan. Given join cost estimates, this problem is NP-Hard making a parallel query optimizer desirable.

## Installation Instructions

## Examples

## Algorithm

## Comparison with Postgres

## Known Bugs and Future Work

## References
