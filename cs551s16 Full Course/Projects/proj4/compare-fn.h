#ifndef _COMPARE_FN_H
#define _COMPARE_FN_H

/** Return 0 if *p1 == *p2, >0 if *p1 > *p2, <0 if *p1 < *p2.
 *  If context if not NULL, then specify a context for the comparison.
 */
typedef int CompareFn(const void *p1, const void *p2, void *context);

#endif //ifndef _COMPARE_FN_H
