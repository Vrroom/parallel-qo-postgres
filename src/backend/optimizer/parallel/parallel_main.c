#include "postgres.h"
#include "optimizer/parallel_worker.h"
#include "optimizer/parallel_tree.h"
#include "optimizer/parallel_eval.h"
#include "optimizer/parallel.h"
#include <pthread.h>
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
	// Array of threads to refer back to while joining.
	pthread_t * threads = (pthread_t *) palloc(n_workers * sizeof(pthread_t));
	// The individual worker information that needs to be passed.
	WorkerData * items = (WorkerData *) palloc(n_workers * sizeof(WorkerData));
	// To ensure reliable concurrency, we will pass a copy of the
	// root to each worker. Now each worker may/may not modify
	// this copy. Finally the output returned by each worker
	// is a tuple of PlannerInfo * and RelOptInfo *.
	// If the RelOptInfo found by this worker is indeed the best
	// then we copy back its PlannerInfo back into the root.
	// This may not be necessary if the worker doesn't modify
	// the PlannerInfo but we don't know that.
	for(int i = 0; i < n_workers; i++){

		// Add relevant info for this worker.
		items[i].root = root;
		items[i].levels_needed = levels_needed;
		items[i].initial_rels = initial_rels;
		items[i].part_id = i;
		items[i].n_workers = n_workers;
		items[i].p_type = p_type;

        // int success = pthread_create(&threads[i], NULL, worker, &items[i]);
		// Assert(success == 0);
	}
	// ParallelPlan * best = (ParallelPlan *) palloc(sizeof(ParallelPlan));
	// int join_success = pthread_join(threads[0], &best);
	ParallelPlan * best = worker(&items[0]);

	ParallelPlan * optimal = best;
	// Assert(join_success == 0);
	// Join threads and extract individual results.
	// Set the best path.
	for(int i = 1; i < n_workers; i++){
		// ParallelPlan * that = (ParallelPlan *) palloc (sizeof(ParallelPlan));
		// join_success = pthread_join(threads[i], &that);
		// Assert(join_success == 0);
		ParallelPlan * that = worker(&items[i]);
		if (that->cost < optimal->cost) optimal = that;
	}
	MemoryContextSwitchTo(oldcxt);
 	MemoryContextDelete(mycontext);
	return construct_rel_based_on_plan(root, 
			levels_needed, initial_rels, optimal->root);
}
