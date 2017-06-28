#ifndef _ARRAYI_H
#define _ARRAYI_H

#include "compare-fn.h"
#include "interfaces.h"

#include <stddef.h>

/** A array which provided a mapping from non-negative array indexes
 *  to array elements.  Note that the sizeof an array element is
 *  specified in the constructor of the implementing class.  It is
 *  assumed that array elements can be copied using a bitwise copy.
 *
 *  The mapping may not be constant, i.e. elements could move within
 *  the array; this is obviously true after a sort, but depending
 *  on the implementation, unrelated elements could move after
 *  a new element is added.
 */

/** Array interface declaration */
typedef struct ArrayInterface ArrayI;

/** Generic array element pointer (the element is the pointed-to value, not
 *  the pointer.
 */
typedef void *ArrayElementP;

/** Declare all functions exposed via the array interface */
struct ArrayFns {

  /** Return element at array[index], NULL if index out-of-bounds */
  ArrayElementP (*get)(const ArrayI *array, int index);

  /** Append element to array.  Return index of added element. Note
   *  that depending on the array implementation or constructor
   *  options, the element may or may not be added at the end of the
   *  array.  If element could not be added (possibly a fixed sized
   *  array which is full), then the return value is < 0.
   */
  int (*add)(ArrayI *array, ArrayElementP element);

  /** Replace array[index] with element, returning stored element
   *  pointer.  Return NULL if index not previously stored.
   */
  ArrayElementP (*put)(ArrayI *array, ArrayElementP element, int index);

  /** Return number of elements stored in array. */
  size_t (*size)(const ArrayI *array);

  /** If array is of fixed size, return max. # of elements which
   *  can be stored in the array.  Return < 0 if there is no
   *  limit on the number of elements.
   */
  int (*max_size)(const ArrayI *array);

  /** Trim array to nElements (no effect if nElements > size()) */
  void (*trim)(ArrayI *array, size_t nElements);

  /** Clear array.  Equivalent to trim(array, 0). */
  void (*clear)(ArrayI *array);

  /** Swap array elements at indexes i and j.  Assumes that
   *  indexes in range.
   */
  void (*swap)(ArrayI *array, int i, int j);

  /** Sort array using compareFn specified as array option. Returns < 0
   *  with no effect if no compareFn was specified as array option.
   */
  int (*sort)(ArrayI *array);

  /** Sort array using specified compareFn. */
  void (*sort_with_compare)(ArrayI *array, CompareFn *compareFn);

  /** Free all resources used by array */
  void (*free)(ArrayI *array);
};

/** Options for array constructors */
typedef struct {

  /** If >= 0, then the number of elements in the array cannot be
   *  greater than this specified value.  Depending on the
   *  implementation or array options, elements added after the array
   *  has reached this size may not be added or may result in some
   *  other array element added earlier being evicted.
   */
  int maxNElements;

  /** If non-null, then array elements may be moved as successive elements
   *  are added in order to facilitate a subsequent sort().
   */
  CompareFn *compareFn;

} ArrayOptions;

struct ArrayInterface {
  const struct ArrayFns *FUNCTIONS_I;
};

#endif //ifndef _ARRAYI_H
