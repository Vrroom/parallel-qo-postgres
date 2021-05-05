#include "postgres.h"
#include "optimizer/parallel_worker.h"
#include "optimizer/parallel_tree.h"
#include "optimizer/parallel_eval.h"
#include "optimizer/parallel.h"
#include <sys/types.h>
#include <unistd.h>
#include "utils/memutils.h"

/**
 * Find the optimal plan for a query.
 *
 * Based on the ideas in the paper:
 *
 * 		Parallelizing Query Optimization
 * 		on Shared-Nothing Architectures.
 *
 * Finding the optimal query plan involves evaluating 
 * plans in a vast space of possible plans. This problem 
 * is NP Hard in general. This algorithm breaks down the 
 * space of plans into subspaces where the optimal plan 
 * in each subspace can be determined parallely. This
 * routine then combines the results of the subspaces
 * to determine the optimal plan under the current table 
 * cost statistics. 
 */
RelOptInfo *
parallel_join_search(
		PlannerInfo *root, 
		int levels_needed, 
		List * initial_rels, 
		int n_workers, 
		int p_type)
{
    MemoryContext mycontext;
	MemoryContext oldcxt;
	mycontext = AllocSetContextCreate(CurrentMemoryContext,
									  "PARALLEL_JOIN_SEARCH",
									  ALLOCSET_DEFAULT_SIZES);
	oldcxt = MemoryContextSwitchTo(mycontext);
	WorkerData * items = (WorkerData *) palloc(n_workers * sizeof(WorkerData));
	for(int i = 0; i < n_workers; i++){
		// Add relevant info for this worker.
		items[i].root = root;
		items[i].levels_needed = levels_needed;
		items[i].initial_rels = initial_rels;
		items[i].part_id = i;
		items[i].n_workers = n_workers;
		items[i].p_type = p_type;
	}
	ParallelPlan * best = worker(&items[0]);
	// Set the best path.
	for(int i = 1; i < n_workers; i++){
		ParallelPlan * that = worker(&items[i]);
	 	if (that->cost < best->cost) best = that;
	}
	MemoryContextSwitchTo(oldcxt);
	RelOptInfo * rel = construct_rel_based_on_plan(root, levels_needed, initial_rels, best->root);
	MemoryContextDelete(mycontext);
	return rel;
}
