#ifndef _LINKED_LIST_MAP_H
#define _LINKED_LIST_MAP_H

#include "compare-fn.h"
#include "mapi.h"

/** Return a new linkeded list map with specified fn used for comparing keys */
MapI *new_linked_list_map(CompareFn *fn);

#endif //ifndef _LINKED_LIST_MAP_H
