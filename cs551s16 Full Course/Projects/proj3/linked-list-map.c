#include "compare-fn.h"
#include "linked-list-map.h"
#include "memalloc.h"

typedef struct NodeStruct {
  MapEntry entry;
  struct NodeStruct *succ;
} Node;

typedef struct {
  MapI map;                /** interface */
  CompareFn *isCompare;    /** comparison function */
  size_t size;             /** # of stored map entries */
  Node *nodes;             /** head of singly-linked list */
} LinkedList;

static MapValue
get_linked_list(const LinkedList *list, MapKey key)
{
  for (Node *p = list->nodes; p != NULL; p = p->succ) {
    if (list->isCompare(p->entry.key, key, (void *)list) == 0) {
      return p->entry.value;
    }
  }
  return NULL;
}


static inline MapValue
get(const MapI *map, MapKey key)
{
  return get_linked_list((LinkedList *)map, key);
}

static void
put_linked_list(LinkedList *list, MapKey key, MapValue value)
{
  for (Node *p = list->nodes; p != NULL; p = p->succ) {
    if (list->isCompare(p->entry.key, key, list) == 0) {
      p->entry.value = value;
      return;
    }
  }
  Node *node = mallocChk(sizeof(Node));
  node->entry.key = key; node->entry.value = value;
  node->succ = list->nodes;
  list->nodes = node;
  list->size++;
}

static inline void
put(MapI *map, MapKey key, MapValue value)
{
  put_linked_list((LinkedList *)map, key, value);
}

static size_t
size_linked_list(const LinkedList *list) {
  return list->size;
}

/** Return number of entries in map. */
static inline size_t
size(const MapI *map)
{
  return size_linked_list((const LinkedList *)map);
}


static void
clear_linked_list(LinkedList *list)
{
  Node *last;
  for (Node *p = list->nodes; p != NULL; p = last) {
    last = p->succ;
    free(p);
  }
  list->size = 0;
  list->nodes = NULL;
}

/** Clear out all map entries. */
static inline void
clear(MapI *map)
{
  clear_linked_list((LinkedList *)map);
}




static const MapEntry *
iterate_linked_list(const LinkedList *list, const MapEntry *lastEntry)
{
  if (lastEntry == NULL) {
    return (list->nodes == NULL) ? NULL : &list->nodes->entry;
  }
  else {
    Node *succ = ((Node *)lastEntry)->succ;
    return (succ == NULL) ? NULL : &succ->entry;
  }
}

static inline const MapEntry *
iterate(const MapI *map, const MapEntry *lastEntry)
{
  return iterate_linked_list((const LinkedList *)map, lastEntry);
}

static void
free_linked_list(LinkedList *list)
{
  clear_linked_list(list);
  free(list);
}

static inline void
free_map(MapI *map)
{
  free_linked_list((LinkedList *)map);
}

static struct MapFns fns = {
  .get = get,
  .put = put,
  .size = size,
  .clear = clear,
  .iterate = iterate,
  .free = free_map
};


/** Return a new linked list map with specified fn used for comparing keys */
MapI *
new_linked_list_map(CompareFn *fn)
{
  LinkedList *list = callocChk(1, sizeof(LinkedList));
  list->map.FUNCTIONS_I = &fns;
  list->isCompare = fn;
  list->size = 0;
  list->nodes = NULL;
  return &list->map;
}

#ifdef TEST_LINKED_LIST_MAP

#include "map-unit-test.h"
#include "unit-test.h"

#include <stdio.h>
#include <string.h>

static int
string_compare(const void *p1, const void *p2, void *context)
{
  return strcmp(p1, p2);
}

int
main(int argc, const char *argv[])
{
  MapI *map = new_linked_list_map(string_compare);
  map_unit_test(map, 1);
  CALL_I(free, map);
  report_unit_test(stderr);
  return (num_fail_unit_test() != 0);
}

#endif //ifdef TEST_LINKED_LIST_MAP
