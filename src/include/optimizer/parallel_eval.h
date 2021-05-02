#ifndef PARALLEL_EVAL_H
#define PARALLEL_EVAL_H

#include "optimizer/parallel.h"
#include "optimizer/parallel_tree.h"

/* Based on geqo_eval.c */
extern double parallel_eval (
	PlannerInfo *root, 
	int levels_needed,
	List * initial_rels,
	BinaryTree * bt
);

extern RelOptInfo * construct_rel_based_on_plan (
	PlannerInfo * root, 
	int levels_needed,
	List * initial_rels,
	BinaryTree * bt
);

#endif
