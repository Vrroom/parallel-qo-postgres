#ifndef PARALLEL_TREE_H
#define PARALLEL_TREE_H

#include "optimizer/parallel.h"

typedef struct BinaryTree
{
	struct BinaryTree *left;
	struct BinaryTree *right;
	List * relids;
} BinaryTree;

typedef struct 
{
	BinaryTree *root;
	double cost;
} ParallelPlan;

extern BinaryTree * create_leaf (int relid);
extern BinaryTree * merge (BinaryTree * l, BinaryTree *r);
extern ParallelPlan * create_parallel_plan (BinaryTree * root, double cost);
extern int tree_2_bitmap (BinaryTree * bt);
extern bool is_leaf (BinaryTree *bt); 

#endif
