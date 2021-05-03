#ifndef PARALLEL_UTILS_H
#define PARALLEL_UTILS_H

#include "optimizer/parallel.h"

extern List * deep_copy_list_of_list_of_ints (List *);
extern List * copy_concat_int(List *, List *);
extern List * cartesian_product(List *, List *);

#endif
