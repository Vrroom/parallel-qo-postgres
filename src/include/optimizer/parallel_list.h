#ifndef PARALLEL_LIST_H
#define PARALLEL_LIST_H

#include "optimizer/parallel.h"

extern List * list_concat_p (List *, List *);
extern List * list_copy_p (List *);
extern int list_length_p (List *);

extern List * list_nth_p (List *, int);
extern int list_nth_int_p (List *, int);

extern List * lappend_p (List *, List *);

extern List * list_make2_int_p(int, int);

#endif
