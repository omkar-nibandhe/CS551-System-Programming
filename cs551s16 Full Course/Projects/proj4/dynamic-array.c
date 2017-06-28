#define _GNU_SOURCE 1

#include "arrayi.h"
#include "compare-fn.h"
#include "memalloc.h"
#include "dynamic-array.h"
#include "heapsort.h"

#include <assert.h>
#include <string.h>

/** A array which is grown dynamically as needed.  Note that the
 *  sizeof an array element is specified in the constructor of the
 *  implementing class.  It is assumed that array elements can
 *  be copied using a bitwise copy.
 */

typedef struct {
  ArrayI array;         /** interface */
  size_t elementSize;   /** # of bytes in each element */
  size_t log2RowSize;   /** log2(# of elements in each row) */
  size_t rowInc;        /** # of row pointers allocated at a time */
  size_t nRows;         /** # of allocated rows */
  size_t nElements;     /** # of assigned elements */
  int maxNElements;     /** max # of elements: if >= 0 then fixed-sized array */
  CompareFn *compareFn; /** function used to compare elements */
  char **rows;          /** dynamically allocated row pointers */
} DynamicArray;

typedef struct {
  int rowIndex;
  int colIndex;
} Coords;

static Coords
split_index(const DynamicArray *array, int index)
{
  Coords c;
  c.rowIndex = index >> array->log2RowSize;
  c.colIndex = index & ((1 << array->log2RowSize) - 1);
  return c;
}

static inline int
is_full(const DynamicArray *array)
{
  return (array->maxNElements >= 0 && array->nElements >= array->maxNElements);
}

/** Return element at array[index], NULL if index out-of-bounds */
static ArrayElementP
get_dynamic_array(const DynamicArray *array, int index)
{
  if (index >= array->nElements) return NULL;
  Coords c = split_index(array, index);
  return &array->rows[c.rowIndex][c.colIndex * array->elementSize];
}

/** Return element at array[index], NULL if index out-of-bounds */
static inline ArrayElementP
get(const ArrayI *array, int index)
{
  return get_dynamic_array((const DynamicArray *)array, index);
}

/** Add element to array.  Return index of added element. */
static int
append_dynamic_array(DynamicArray *array, ArrayElementP element)
{
  assert(!is_full(array));
  Coords c = split_index(array, array->nElements);
  if (c.rowIndex >= array->nRows) {
    size_t newNRows = array->nRows + array->rowInc;
    array->rows = reallocChk(array->rows, newNRows*sizeof(char *));
    for (int i = array->nRows; i < newNRows; i++) {
      array->rows[i] = NULL;
    }
    array->nRows = newNRows;
  }
  if (array->rows[c.rowIndex] == NULL) {
    array->rows[c.rowIndex] =
      mallocChk(array->elementSize * (1 << array->log2RowSize));
  }
  char *destAddr = &array->rows[c.rowIndex][c.colIndex * array->elementSize];
  memcpy(destAddr, element, array->elementSize);
  int index = array->nElements++;
  return index;
}

/** Insert element in sorted (<=) array.  Return index of added element. */
static int
insert_dynamic_array(DynamicArray *array, ArrayElementP element)
{
  int nElements0 = array->nElements;
  ArrayElementP lastElement =
    (nElements0 > 0) ? get_dynamic_array(array, nElements0 - 1) : NULL;
  int doAppend =
    lastElement == NULL || array->compareFn(element, lastElement, array) > 0;
  int isFull = is_full(array);
  if (doAppend) {
    return (isFull) ? -1 : append_dynamic_array(array, element);
  }
  //will insert within existing elements
  if (!isFull && lastElement != NULL) { //copy lastElement to new entry
    append_dynamic_array(array, lastElement);
  }
  int i;
  for (i = nElements0 - 1; i > 0; i--) {
    //element at index i has been copied or is being discarded
    ArrayElementP i0Element = get_dynamic_array(array, i - 1);
    if (array->compareFn(i0Element, element, array) < 0) {
      break;
    }
    else {
      //copy element at index i - 1 into element i
      ArrayElementP iElement = get_dynamic_array(array, i);
      memcpy(iElement, i0Element, array->elementSize);
    }
  }
  ArrayElementP iElement = get_dynamic_array(array, i);
  memcpy(iElement, element, array->elementSize);
  return i;
}

/** Add element to array.  Return index of added element. */
static int
add_dynamic_array(DynamicArray *array, ArrayElementP element)
{
  if (array->compareFn != NULL) { //array is sorted in non-descending order
    return insert_dynamic_array(array, element);
  }
  return (is_full(array)) ? -1 : append_dynamic_array(array, element);
}

/** Add element to array.  Return index of added element. */
static inline int
add(ArrayI *array, ArrayElementP element)
{
  return add_dynamic_array((DynamicArray *)array, element);
}

/** Replace array[index] with element, returning stored element
 *  pointer.  Return NULL if index not previously stored.
 */
static ArrayElementP
put_dynamic_array(DynamicArray *array, ArrayElementP element, int index)
{
  if (index >= array->nElements) return NULL;
  Coords c = split_index(array, index);
  char *destAddr = &array->rows[c.rowIndex][c.colIndex * array->elementSize];
  memcpy(destAddr, element, array->elementSize);
  return destAddr;
}

/** Replace array[index] with element, returning stored element
 *  pointer.  Return NULL if index not previously stored.
 */
static inline ArrayElementP
put(ArrayI *array, ArrayElementP element, int index)
{
  return put_dynamic_array((DynamicArray *)array, element, index);
}

/** Return number of elements stored in array. */
static size_t
size_dynamic_array(const DynamicArray *array)
{
  return array->nElements;
}

/** Return number of elements stored in array. */
static inline size_t
size(const ArrayI *array)
{
  return size_dynamic_array((const DynamicArray *)array);
}

/** If array is of fixed size, return max. # of elements which
 *  can be stored in the array.  Return < 0 if there is no
 *  limit on the number of elements.
 */
static inline int
max_size_dynamic_array(const DynamicArray *array)
{
  return array->maxNElements;
}

/** If array is of fixed size, return max. # of elements which
 *  can be stored in the array.  Return < 0 if there is no
 *  limit on the number of elements.
 */
static inline int max_size(const ArrayI *array) {
  return max_size_dynamic_array((DynamicArray *)array);
}

/** Trim array to nElements (no effect if nElements > size()) */
static void
trim_dynamic_array(DynamicArray *array, size_t nElements)
{
  if (nElements >= array->nElements) return;
  Coords c = split_index(array, nElements);
  for (int i = c.rowIndex + 1; i < array->nRows; i++) {
    free(array->rows[i]);
    array->rows[i] = NULL;
  }
  array->nElements = nElements;
}

/** Trim array to nElements (no effect if nElements > size()) */
static inline void
trim(ArrayI *array, size_t nElements) {
  trim_dynamic_array((DynamicArray *)array, nElements);
}

/** Reset array.  Equivalent to trim(0). */
static inline void clear(ArrayI *array) { trim(array, 0); }

/** Swap array elements at indexes i and j.  Assumes that
 *  indexes in range.
 */
static void
swap_dynamic_array(DynamicArray *array, int i, int j)
{
  char tmp[array->elementSize];
  memcpy(tmp, get_dynamic_array(array, i), array->elementSize);
  put_dynamic_array(array, get_dynamic_array(array, j), i);
  put_dynamic_array(array, tmp, j);
}


/** Swap array elements at indexes i and j.  Assumes that
 *  indexes in range.
 */
static inline void
swap(ArrayI *array, int i, int j)
{
  swap_dynamic_array((DynamicArray*)array, i, j);
}

/** Sort array using compareFn specified as array option. Returns < 0
 *  with no effect if no compareFn was specified as array option.
 */
static int
sort_dynamic_array(DynamicArray *array)
{
  if (array->compareFn == NULL) return -1;
  heapsort((ArrayI *)array, array->compareFn);
  return 0;
}

/** Sort array using compareFn specified as array option. Returns < 0
 *  with no effect if no compareFn was specified as array option.
 */
static inline int
sort(ArrayI *array)
{
  return sort_dynamic_array((DynamicArray *)array);
}

/** Sort array using specified compareFn. */
static void
sort_with_compare(ArrayI *array, CompareFn *compareFn)
{
  heapsort(array, compareFn);
}


/** Free all resources used by array */
static void
free_dynamic_array(DynamicArray *array)
{
  for (int i = 0; i < array->nRows; i++) {
    free(array->rows[i]);
  }
  free(array->rows);
  free(array);
}

/** Free all resources used by array */
static inline void
free_array(ArrayI *array)
{
  free_dynamic_array((DynamicArray *)array);
}

static const struct ArrayFns fns = {
  .get = get,
  .add = add,
  .put = put,
  .size = size,
  .max_size = max_size,
  .trim = trim,
  .clear = clear,
  .swap = swap,
  .sort = sort,
  .sort_with_compare = sort_with_compare,
  .free = free_array
};

/** Create a new dynamic array for elements having size elementSize. */
ArrayI *
new_dynamic_array(size_t elementSize, const ArrayOptions *optionsP)
{
  enum { LOG2_ROW_SIZE = 8, ROW_INC = 4 };
  return new_sized_dynamic_array(elementSize, LOG2_ROW_SIZE, ROW_INC, optionsP);
}

static ArrayOptions defaultArrayOptions = {
  .maxNElements = -1,
  .compareFn = NULL
};

/** Create a new dynamic array for elements having size elementSize,
 *  with each having size 2**log2RowSize and number of rowInc rows added
 *  whenever number of rows is sufficient.
 */
ArrayI *
new_sized_dynamic_array(size_t elementSize, size_t log2RowSize, size_t rowInc,
                        const ArrayOptions *optionsP)
{
  if (optionsP == NULL) optionsP = &defaultArrayOptions;
  DynamicArray *array = mallocChk(sizeof(DynamicArray));
  array->array.FUNCTIONS_I = &fns;
  array->elementSize = elementSize;
  array->log2RowSize = log2RowSize;
  array->rowInc = rowInc;
  array->nRows = 0;
  array->nElements = 0;
  array->maxNElements = optionsP->maxNElements;
  array->compareFn = optionsP->compareFn;
  array->rows = NULL;
  return &array->array;
}

#ifdef TEST_DYNAMIC_ARRAY

#include "unit-test.h"

#include <stdlib.h>

typedef int Element;

static inline Element
make_value(int index)
{
  return 4*index + 1;
}


static void
test_unordered(void)
{
  enum { N_ELEMENTS = 4096 };
  size_t elementSize = sizeof(Element);
  ArrayI *array = new_dynamic_array(elementSize, NULL);
  UNIT_TEST(CALL_I(size, array) == 0);
  for (int i = 0; i < N_ELEMENTS; i++) {
    Element value = make_value(i);
    int index = CALL_I(add, array, &value);
    UNIT_TEST(value == *(Element *)CALL_I(get, array, index));
    UNIT_TEST(CALL_I(size, array) == i + 1);
  }
  UNIT_TEST(CALL_I(size, array) == N_ELEMENTS);
  for (int i = 0; i < N_ELEMENTS; i++) {
    Element value = make_value(i);
    Element *stored = CALL_I(get, array, i);
    UNIT_TEST(value == *stored);
  }
  for (int i = 0; i < N_ELEMENTS/2; i++) {
    Element value = i;
    Element *stored = CALL_I(put, array, &value, i);
    UNIT_TEST(value == *stored);
    UNIT_TEST(CALL_I(size, array) == N_ELEMENTS);
  }
  for (int i = 0; i < N_ELEMENTS; i++) {
    Element value = (i < N_ELEMENTS/2) ? i : make_value(i);
    Element *stored = CALL_I(get, array, i);
    UNIT_TEST(value == *stored);
  }
  CALL_I(trim, array, N_ELEMENTS/2);
  for (int i = 0; i < CALL_I(size, array); i++) {
    Element *stored = CALL_I(get, array, i);
    UNIT_TEST(i == *stored);
  }
  CALL_I(clear, array);
  UNIT_TEST(CALL_I(size, array) == 0);
  CALL_I(free, array);
}

static int
integer_compare(const void *p1, const void *p2, void *context)
{
  const int *ip1 = (const int *)p1;
  const int *ip2 = (const int *)p2;
  return *ip1 - *ip2;
}

static void
test_ordered(int maxNElements)
{
  ArrayOptions options = {
    .maxNElements = maxNElements, .compareFn = integer_compare
  };
  ArrayI *array = new_dynamic_array(sizeof(Element), &options);
  enum { DEFAULT_N_ELEMENTS = 500 };
  const int nElements =
    (maxNElements < 0) ? DEFAULT_N_ELEMENTS : maxNElements * 2;
  int elements[nElements];
  for (int i = 0; i < nElements; i++) {
    elements[i] = rand();
    CALL_I(add, array, &elements[i]);
  }
  qsort_r(elements, nElements, sizeof(Element), integer_compare, array);
  int nCheckElements = (maxNElements < 0) ? DEFAULT_N_ELEMENTS : maxNElements;
  for (int i = 1; i < nCheckElements; i++) {
    int *i0 =  CALL_I(get, array, i - 1);
    int *i1 =  CALL_I(get, array, i);
    UNIT_TEST(elements[i - 1] == *i0);
    UNIT_TEST(elements[i] == *i1);
    UNIT_TEST(*i0 <= *i1);
  }
  CALL_I(free, array);
}

int
main(int argc, const char *argv[])
{
  test_unordered();
  test_ordered(-1);
  test_ordered(500);
  report_unit_test(stderr);
  return (num_fail_unit_test() != 0);
}

#endif //ifdef TEST_DYNAMIC_ARRAY
