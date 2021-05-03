#include "postgres.h"
#include "optimizer/parallel_utils.h"

/*
 * Return a deep copy of a list of list of ints.
 */
List * deep_copy_list_of_list_of_ints (List * a) {
	List * result = NIL;
	for (int i = 0; i < list_length(a); i++) {
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
	List * result = list_copy(a);
	result = list_concat(result, list_copy(b));
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
	new_arr = list_concat(new_arr, deep_copy_list_of_list_of_ints(a));
	new_arr = list_concat(new_arr, deep_copy_list_of_list_of_ints(b));
	for(int i = 0; i < list_length(b); i++){
		List * lunion = NIL;
		for(int j = 0; j < list_length(a); j++){
			List * concat = copy_concat_int((List *) list_nth(b, i), (List *) list_nth(a, j));
			lunion = lappend(lunion, concat);
		}
		new_arr = list_concat(new_arr, lunion);
	}

	return new_arr;
}
