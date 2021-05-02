#include "postgres.h"
#include "optimizer/parallel_tree.h"
#include "optimizer/parallel_utils.h"

BinaryTree * create_leaf (int relid) {
	BinaryTree * leaf = (BinaryTree *) palloc(sizeof(BinaryTree));
	leaf->relids = NIL;
	leaf->left = NULL;
	leaf->right = NULL;
	leaf->relids = lappend_int(leaf->relids, relid);
	return leaf;
}

BinaryTree * merge (BinaryTree * l, BinaryTree * r) {
	BinaryTree * node = (BinaryTree *) palloc(sizeof(BinaryTree));
	node->relids = copy_concat_int(l->relids, r->relids);
	node->left = l;
	node->right = r;
	return node;
}

ParallelPlan * create_parallel_plan (BinaryTree * root, double cost) {
	ParallelPlan * p = (ParallelPlan *) palloc(sizeof(ParallelPlan));
	p->root = root;
	p->cost = cost;
	return p;
}

int tree_2_bitmap (BinaryTree * bt) {
	int bitmap = 0;
	for (int i = 0; i < list_length(bt->relids); i++) {
		bitmap |= (1 << (list_nth_int(bt->relids, i)));
	}
	return bitmap;
}

bool is_leaf(BinaryTree * bt) {
	return bt->left == NULL && bt->right == NULL;
}
