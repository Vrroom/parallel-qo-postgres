#include "postgres.h"
#include "optimizer/parallel_list.h"
#include <pthread.h>

pthread_mutex_t list_mutex = PTHREAD_MUTEX_INITIALIZER;

List * list_concat_p (List * a, List * b) {
	pthread_mutex_lock(&list_mutex);
	List * result = list_concat(a, b);
	pthread_mutex_unlock(&list_mutex);
	return result;
}

List * list_copy_p (List * a) {
	pthread_mutex_lock(&list_mutex);
	List * result = list_copy(a);
	pthread_mutex_unlock(&list_mutex);
	return result;
}

int list_length_p (List * a) {
	pthread_mutex_lock(&list_mutex);
	int result = list_length(a);
	pthread_mutex_unlock(&list_mutex);
	return result;
}

List * list_nth_p (List * a, int i) {
	pthread_mutex_lock(&list_mutex);
	List * result = list_nth(a, i);
	pthread_mutex_unlock(&list_mutex);
	return result;
}

int list_nth_int_p (List * a, int i) {
	pthread_mutex_lock(&list_mutex);
	int result = list_nth_int(a, i);
	pthread_mutex_unlock(&list_mutex);
	return result;
}

List * lappend_p(List *a, List *b) {
	pthread_mutex_lock(&list_mutex);
	List * result = lappend(a, b);
	pthread_mutex_unlock(&list_mutex);
	return result;
}

List * list_make2_int_p(int a, int b) {
	pthread_mutex_lock(&list_mutex);
	List * result = list_make2_int(a, b);
	pthread_mutex_unlock(&list_mutex);
	return result;
}
