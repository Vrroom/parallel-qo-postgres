#include "postgres.h"
#include "optimizer/parallel_worker.h"
#include "optimizer/parallel.h"
#include <pthread.h>

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
	// Array of threads to refer back to while joining.
	pthread_t * threads = (pthread_t *) palloc(n_workers * sizeof(pthread_t));
	// The individual worker information that needs to be passed.
	WorkerData * items = (WorkerData *) palloc(levels_needed * sizeof(WorkerData));
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
		PlannerInfo * root_cpy = (PlannerInfo *) palloc(sizeof(PlannerInfo));
		*root_cpy = *root;

		items[i].root = root_cpy;
		items[i].levels_needed = levels_needed;
		items[i].initial_rels = initial_rels;
		items[i].part_id = i;
		items[i].n_workers = n_workers;
		items[i].p_type = p_type;
		elog(LOG, "Creating Worker Thread - %d", i);
		worker(&items[i]);
		// int success = pthread_create(&threads[i], NULL, worker, &items[i]);
		// Assert(success == 0);
	}
	WorkerOutput * best = (WorkerOutput *) palloc(sizeof(WorkerOutput));
	int join_success = pthread_join(threads[0], &best);

	RelOptInfo * optimal = best->optimal;
	*root = *(best->root);
	Assert(join_success == 0);
	// Join threads and extract individual results.
	// Set the best path.
	for(int i = 1; i < n_workers; i++){
		WorkerOutput * that = (WorkerOutput *) palloc (sizeof(WorkerOutput));
		join_success = pthread_join(threads[i], &that);
		Assert(join_success == 0);

		Path *that_path = that->optimal->cheapest_total_path;
		Path *best_path = best->optimal->cheapest_total_path;

		if(that_path->total_cost < best_path->total_cost){
			optimal = that->optimal;
			*root = *(that->root);
		}
	}
	pfree(items);
	return optimal;
}
