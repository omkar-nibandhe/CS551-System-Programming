#ifndef _HEAPSORT_H
#define _HEAPSORT_H

#include "arrayi.h"
#include "compare-fn.h"

/** Sort array in non-ascending order as per compareFn. */
void heapsort(ArrayI *array, CompareFn *compareFn);

#endif //ifndef _HEAPSORT_H
