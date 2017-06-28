#ifndef _MAPI_H
#define _MAPI_H

#include "interfaces.h"

#include <stddef.h>

/** A map interface */

/** Declare map interface */
typedef struct MapInterface MapI;

/** Generic key; the map stores the pointer, not what is pointed to. */
typedef void *MapKey;

/** Generic value; the map stores the pointer, not what is pointed to. */
typedef void *MapValue;

/** A map-entry is a key-value pair. */
typedef struct {
  MapKey key;
  MapValue value;
} MapEntry;

/** The functions exposed by the map interface. */
struct MapFns {
  /** Return value associated with key in map; NULL if none. */
  MapValue (*get)(const MapI *map, MapKey key);

  /** Set value for key in map. */
  void (*put)(MapI *map, MapKey key, MapKey value);

  /** Return number of entries in map. */
  size_t (*size)(const MapI *map);

  /** Clear out all map entries. */
  void (*clear)(MapI *map);

  /** Iterate over entries in map.  Initially provide lastEntry as NULL and
   *  on successive calls provide return value; quit iteration when
   *  return value is NULL.
   *
   *  MapI map = ...;
   *  for (const MapEntry *entry = map->fns.iterate(map, NULL);
   *       entry != NULL;
   *       entry = map->fns.iterate(map, entry)) {
   *    //do something with *entry
   *  }
   */
  const MapEntry *(*iterate)(const MapI *map, const MapEntry *lastEntry);

  /** Free all resources used by map */
  void (*free)(MapI *map);
};

struct MapInterface {
  const struct MapFns *FUNCTIONS_I;
};


#endif //ifndef _MAPI_H
