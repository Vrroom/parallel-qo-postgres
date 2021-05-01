#ifndef PARALLEL_UTILS_H
#define PARALLEL_UTILS_H

#include "optimizer/parallel.h"

extern List * add_ptrs(List *, List *);
extern List * copy_paste(List *, List *);
extern List * copy_concat_int(List *, List *);
extern List * cartesian_product(List *, List *);

#endif
