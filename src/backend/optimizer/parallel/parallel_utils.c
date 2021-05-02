#include "postgres.h"
#include "optimizer/parallel_utils.h"

/**
 * Concatenate lists a and b into a. Hence, 
 * list a is modified.
 *
 * Example: a = [[1], [2, 3]], b = [[4, 5]]
 * add_ptrs(a, b) = [[1], [2, 3], [4, 5]]
 */
List * add_ptrs(List * a, List * b){
	for(int i = 0; i < list_length(b); i++){
		a = lappend(a, (List *) list_nth(b, i));
	}
	return a;
}

/**
 * Copy a list of lists into result.
 *
 * Example: 
 * result = []
 * a = [[1, 2, 3], [4, 5]]
 * copy_paste(result, a) = result = [[1, 2, 3], [4, 5]]
 */
List * copy_paste(List * result, List * a){
	for(int i = 0; i < list_length(a); i++){
		List * a_cpy = list_copy((List *) list_nth(a, i));
		result = lappend(result, a_cpy);
	}
	return result;
}

/**
 * Concatenate two list of ints into a new list
 * 
 * Example:
 * a = [1, 2, 3]
 * b = [4, 5]
 * copy_concat_int(a, b) = [1, 2, 3, 4, 5] // New List!
 */
List * copy_concat_int(List * a, List * b){
	List * result = NIL;
	for(int i = 0; i < list_length(a); i++){
		result = lappend_int(result, list_nth_int(a, i));
	}
	for(int i = 0; i < list_length(b); i++){
		result = lappend_int(result, list_nth_int(b, i));
	}
	return result;
}

/**
 * Cartesian product of two lists a and b. 
 *
 * The original lists are freed, so always pass a
 * copy. 
 *
 * Example: 
 * a = [1, 2, 3]
 * b = [4, 5]
 * cartesian_product(a, b) = [[1, 4], [1, 5], [2, 4], [2, 5], [3, 4], [3, 5]]
 */
List * cartesian_product(List * a, List * b){
	if(a == NIL)
		return b;
	if(b == NIL)
		return a;
	List * new_arr = NIL;
	new_arr = copy_paste(new_arr, a);
	new_arr = copy_paste(new_arr, b);
	for(int i = 0; i < list_length(b); i++){
		List * lunion = NIL;
		for(int j = 0; j < list_length(a); j++){
			List * concat = copy_concat_int((List *) list_nth(b, i), (List *) list_nth(a, j));
			lunion = lappend(lunion, concat);
		}
		new_arr = add_ptrs(new_arr, lunion);
	}

	return new_arr;
}
