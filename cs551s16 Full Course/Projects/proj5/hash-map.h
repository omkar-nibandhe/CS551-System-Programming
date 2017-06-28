#ifndef _HASH_MAP_H
#define _HASH_MAP_H

#include "compare-fn.h"
#include "mapi.h"

typedef int HashFn(MapKey key);

MapI *new_hash_map(CompareFn *compareFn, HashFn *hashFn);

#endif //ifndef _HASH_MAP_H
