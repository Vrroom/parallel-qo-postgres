#ifndef PARALLEL_H
#define PARALLEL_H

#include "nodes/relation.h"
#include "nodes/nodes.h"

/* routine in parallel_main.c */
extern RelOptInfo *parallel_join_search(
	PlannerInfo *root, 
	int levels_needed, 
	List * initial_rels, 
	int n_workers, 
	int p_type);

#endif
