#include "arrayi.h"
#include "compare-fn.h"
#include "heapsort.h"


//Adapted from <https://en.wikipedia.org/wiki/Heapsort>

static inline int leftChildIndex(int i) { return 2 * i + 1; }
static inline int rightChildIndex(int i) { return 2 * i + 2; }
static inline int parentIndex(int i) { return (i - 1)/2; }

/** Repair heap array[lo..hi]; array[lo] may violate the heap
 *  property, but its children are valid heaps.
 */
static void
sift_down(ArrayI *array, CompareFn *compareFn, int lo, int hi)
{
  while (leftChildIndex(lo) <= hi) { //lo has at least one child
    void *loElement = CALL_I(get, array, lo);
    //Set maxElement to max of array[lo] and its 2 kids and
    //maxIndex to it index of the maxElement.
    void *maxElement = loElement;
    int maxIndex = lo;
    int loLeftIndex = leftChildIndex(lo);
    void *loLeftChild = CALL_I(get, array, loLeftIndex);
    if (compareFn(loLeftChild, maxElement, array) > 0) { //left child > a[lo]
      maxIndex = loLeftIndex;
      maxElement = loLeftChild;
    }
    int loRightIndex = rightChildIndex(lo);
    if (loRightIndex <= hi) {
      void *loRightChild = CALL_I(get, array, loRightIndex);
      if (compareFn(loRightChild, maxElement, array) > 0) { //right kid is max
        maxIndex = loRightIndex;
        maxElement = loRightChild;
      }
    }
    if (maxIndex == lo) { //heap restored
      break;
    }
    CALL_I(swap, array, maxIndex, lo);
    lo = maxIndex;
  }
}

/** Rearrange array elements so that they have the heap property.
 */
static void
heapify(ArrayI *array, CompareFn *compareFn)
{
  int lastIndex = CALL_I(size, array) - 1;
  for (int start = parentIndex(lastIndex); start >= 0; start--) {
    //restore heap rooted at start
    sift_down(array, compareFn, start, lastIndex);
  }
}

/** Sort array in non-ascending order as per compareFn. */
void
heapsort(ArrayI *array, CompareFn *compareFn)
{
  heapify(array, compareFn); //Build max heap with largest value at [0]
  for (int end = CALL_I(size, array) - 1; end > 0; end--) {
    CALL_I(swap, array, 0, end);
    sift_down(array, compareFn, 0, end - 1);
  }
}

#ifdef TEST_HEAPSORT

#include "dynamic-array.h"
#include "unit-test.h"

#include <stdlib.h>

static int
compare_ints(const void *p1, const void *p2, void *context)
{
  const int *iP1 = (int *)p1;
  const int *iP2 = (int *)p2;
  return *iP1 - *iP2;
}

static void
check_ordered(ArrayI *array)
{
  int nElements = CALL_I(size, array);
  for (int i = 0; i < nElements - 1; i++) {
    int *v = CALL_I(get, array, i);
    int *v1 = CALL_I(get, array, i + 1);
    UNIT_TEST(*v <= *v1);
  }
}

static void
test_sort(int n)
{
  ArrayI *array = new_dynamic_array(sizeof(int), NULL);
  for (int i = 0; i < n; i++) {
    int v = rand() % 99999;
    CALL_I(add, array, &v);
  }
  heapsort(array, compare_ints);
  check_ordered(array);
  CALL_I(free, array);
}

int
main(int argc, const char *argv[])
{
  test_sort(10);
  test_sort(0);
  //test_sort(4096);
  report_unit_test(stderr);
  return (num_fail_unit_test() != 0);
}

#endif //ifdef TEST_HEAPSORT
