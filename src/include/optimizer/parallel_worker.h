#ifndef PARALLEL_WORKER_H
#define PARALLEL_WORKER_H

#include "optimizer/parallel.h"
#include "optimizer/parallel_tree.h"

/* data passed to each worker thread */
typedef struct
{
	PlannerInfo * root;  /* data structure containing plan info */
	List * initial_rels; /* list of jointree items */
	int levels_needed;   /* number of initial jointree items in query*/
	int part_id;         /* which partition is current worker dealing with */
	int n_workers;       /* total number of workers */ 
	int p_type;          /* type of plan, linear (2) or bushy (3) */
} WorkerData;

extern int ptr_less(const void *, const void *);
extern List * constrained_power_set(List *, int, int);
extern List * part_constraints(int, int, int);
extern List * adm_join_results(int, List *);
extern void try_splits(PlannerInfo *, int, List *, List *, List *, ParallelPlan **);
extern void * worker(void *);

#endif
