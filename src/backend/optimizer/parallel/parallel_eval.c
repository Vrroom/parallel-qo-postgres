#include "postgres.h"

#include <float.h>
#include <limits.h>
#include <math.h>
#include <pthread.h>

#include "optimizer/joininfo.h"
#include "optimizer/pathnode.h"
#include "optimizer/paths.h"
#include "optimizer/parallel_eval.h"
#include "optimizer/parallel_tree.h"
#include "utils/memutils.h"

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER; 

/* A "clump" of already-joined relations within construct_rel_based_on_plan */
typedef struct
{
	RelOptInfo *joinrel;		/* joinrel for the set of relations */
	int			size;			/* number of input relations in clump */
} Clump;

static List *
merge_clump(
	PlannerInfo *root, 
	int levels_needed,
	List *initial_rels,
	BinaryTree * bt, 
	bool force);

static bool desirable_join(PlannerInfo *root,
			   RelOptInfo *outer_rel, RelOptInfo *inner_rel);


double parallel_eval (
		PlannerInfo * root, 
		int levels_needed,
		List * initial_rels,
		BinaryTree * bt)
{
	pthread_mutex_lock(&mutex);
	MemoryContext mycontext;
	MemoryContext oldcxt;
	RelOptInfo *joinrel;
	double		cost;
	int			savelength;
	struct HTAB *savehash;

	mycontext = AllocSetContextCreate(CurrentMemoryContext,
									  "PARALLEL",
									  ALLOCSET_DEFAULT_SIZES);
	oldcxt = MemoryContextSwitchTo(mycontext);
	savelength = list_length(root->join_rel_list);
	savehash = root->join_rel_hash;
	Assert(root->join_rel_level == NULL);
	root->join_rel_hash = NULL;
	/* construct the best path for the given combination of relations */
	joinrel = construct_rel_based_on_plan(root,
 		   levels_needed, initial_rels, bt);

	if (joinrel) {
		Path *best_path = joinrel->cheapest_total_path;
		cost = best_path->total_cost;
	} else cost = DBL_MAX;

	/*
	 * Restore join_rel_list to its former state, and put back original
	 * hashtable if any.
	 */
	root->join_rel_list = list_truncate(root->join_rel_list,
										savelength);
	root->join_rel_hash = savehash;
	MemoryContextSwitchTo(oldcxt);
	MemoryContextDelete(mycontext);
	pthread_mutex_unlock(&mutex);

	return cost;
}

RelOptInfo *
construct_rel_based_on_plan(
	PlannerInfo *root, 
	int levels_needed,
	List * initial_rels,
	BinaryTree * bt)
{
	List * relList = NIL;

	/*
	 * Sometimes, a relation can't yet be joined to others due to heuristics
	 * or actual semantic restrictions. 
	 */
	relList = merge_clump(root, levels_needed, initial_rels, bt, true);
	// TODO : REPLACE WITH THE PROPER ALGORITHM.
///	if (list_length(clumps) > 1) {
///		/* Force-join the remaining clumps in some legal order */
///		List	   *fclumps;
///		ListCell   *lc;
///
///		fclumps = NIL;
///		foreach(lc, clumps)
///		{
///			Clump	   *clump = (Clump *) lfirst(lc);
///
///			fclumps = merge_clump(root, fclumps, clump, num_gene, true);
///		}
///		clumps = fclumps;
///	}
///
	/* Did we succeed in forming a single join relation? */
	if (list_length(relList) != 1)
		return NULL;

	return (RelOptInfo *) linitial(relList);
}

static List *
merge_clump(
	PlannerInfo *root, 
	int levels_needed,
	List *initial_rels,
	BinaryTree * bt, 
	bool force)
{
	List * relList = NIL;
	if (is_leaf(bt)) {
		int relid = linitial_int(bt->relids);
		RelOptInfo * rel = list_nth(initial_rels, relid);
		relList = lappend(relList, rel);
	} else {
		List * list1 = merge_clump(root, levels_needed, 
				initial_rels, bt->left, force);
		List * list2 = merge_clump(root, levels_needed, 
				initial_rels, bt->right, force);
		/* 
 		 * It is possible that these are lists with more than one
 		 * element in either of them. If so, then just concat and
 		 * return the result. Else, take the singleton elements
 		 * in the lists and try to join them
 		 */
		if (list_length(list1) > 1 || list_length(list2) > 1) { 
			relList = list_concat(list1, list2);
		} else {
			Assert(list_length(list1) == 1 && list_length(list2) == 1);
			RelOptInfo * rel1 = linitial(list1);
			RelOptInfo * rel2 = linitial(list2);
			if (force || desirable_join(root, rel1, rel2)) {
				RelOptInfo *joinrel;
				joinrel = make_join_rel(root, rel1, rel2);
				if (joinrel) {
					generate_partitionwise_join_paths(root, joinrel);
					if (list_length(bt->relids) < levels_needed)
						generate_gather_paths(root, joinrel, false);
					/* Find and save the cheapest paths for this joinrel */
					set_cheapest(joinrel);
					relList = lappend(relList, joinrel);				
				} else {
					relList = list_concat(list1, list2);
				}
			}
		}
	}
	return relList;
}

/*
 * Heuristics for gimme_tree: do we want to join these two relations?
 */
static bool
desirable_join(PlannerInfo *root,
			   RelOptInfo *outer_rel, RelOptInfo *inner_rel)
{
	/*
	 * Join if there is an applicable join clause, or if there is a join order
	 * restriction forcing these rels to be joined.
	 */
	if (have_relevant_joinclause(root, outer_rel, inner_rel) ||
		have_join_order_restriction(root, outer_rel, inner_rel))
		return true;

	/* Otherwise postpone the join till later. */
	return false;
}
