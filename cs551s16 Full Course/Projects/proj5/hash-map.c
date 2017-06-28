#include "arrayi.h"
#include "dynamic-array.h"
#include "hash-map.h"
#include "memalloc.h"

#include <assert.h>

typedef struct {
  MapEntry entry;
  int succ;
} Node;

typedef struct {
  MapI map;                /** interface */
  CompareFn *isCompare;    /** comparison function */
  HashFn *hashFn;          /** hashcode function */
  ArrayI *nodes;           /** storage for nodes */
  size_t hashtableSize;    /** # hashtable entries */
  int *hashtable;          /** actual hash-table */
} HashMap;

/** Return next prime greater than v */
static int
next_prime(int v)
{
  if (v < 2) return 2;
  int candidate = v + v%2 + 1;
  assert(candidate % 2 == 1);
  do {
    int isPrime = 1;
    for (int i = 3; i*i <= candidate; i += 2) {
      if (candidate % i == 0) {
        isPrime = 0;
        break;
      }
    }
    if (isPrime) break;
    candidate += 2;
  } while (1);
  return candidate;
}

static inline
int hash_index(const HashMap *map, MapKey key)
{
  return map->hashFn(key) % map->hashtableSize;
}

static void
rehash(HashMap *map)
{
  enum { INIT_SIZE = 73 };
  size_t hashtableSize =
    (map->hashtableSize > 0)
    ? next_prime(2 * map->hashtableSize)
    : INIT_SIZE;
  map->hashtable = reallocChk(map->hashtable, hashtableSize * sizeof(int));
  map->hashtableSize = hashtableSize;
  for (int i = 0; i < hashtableSize; i++) {
    map->hashtable[i] = -1;
  }
  int nNodes = CALL_I(size, map->nodes);
  for (int i = 0; i < nNodes; i++) {
    Node *p = CALL_I(get, map->nodes, i);
    int index = hash_index(map, p->entry.key);
    p->succ = map->hashtable[index];
    map->hashtable[index] = i;
  }
}

static MapValue
get_hash_map(const HashMap *map, MapKey key)
{
  int next;
  for (int index = map->hashtable[hash_index(map, key)]; index >= 0;
       index = next) {
    const Node *p = CALL_I(get, map->nodes, index);
    if (map->isCompare(p->entry.key, key, (void *)map) == 0) {
      return p->entry.value;
    }
    next = p->succ;
  }
  return NULL;
}


static inline MapValue
get(const MapI *map, MapKey key)
{
  return get_hash_map((HashMap *)map, key);
}

static void
put_hash_map(HashMap *map, MapKey key, MapValue value)
{
  int *chainHead = &map->hashtable[hash_index(map, key)];
  int next;
  for (int index = *chainHead; index >= 0; index = next) {
    Node *p = CALL_I(get, map->nodes, index);
    if (map->isCompare(p->entry.key, key, map) == 0) {
      p->entry.value = value;
      return;
    }
    next = p->succ;
  }
  Node node = { .entry.key = key, .entry.value = value, .succ = *chainHead };
  *chainHead = CALL_I(add, map->nodes, &node);
  if (CALL_I(size, map->nodes) > 4 * map->hashtableSize) {
    rehash(map);
  }
}

static inline void
put(MapI *map, MapKey key, MapValue value)
{
  put_hash_map((HashMap *)map, key, value);
}

static size_t
size_hash_map(const HashMap *map)
{
  return CALL_I(size, map->nodes);
}

/** Return number of entries in map. */
static inline size_t
size(const MapI *map)
{
  return size_hash_map((const HashMap *)map);
}


static void
clear_hash_map(HashMap *map)
{
  CALL_I(clear, map->nodes);
  for (int i = 0; i < map->hashtableSize; i++) {
    map->hashtable[i] = -1;
  }
}

/** Clear out all map entries. */
static inline void
clear(MapI *map)
{
  clear_hash_map((HashMap *)map);
}

/** Return first node on next non-empty hash-chain after index */
static Node *
next_chain_node(const HashMap *map, int index)
{
  for (int i = (index < 0) ? 0 : index + 1; i < map->hashtableSize; i++) {
    if (map->hashtable[i] >= 0) {
      return CALL_I(get, map->nodes, map->hashtable[i]);
    }
  }
  return NULL;
}

static const MapEntry *
iterate_hash_map(const HashMap *map, const MapEntry *lastEntry)
{
  if (lastEntry == NULL) {
    Node *p = next_chain_node(map, -1);
    return (p == NULL) ? NULL : &p->entry;
  }
  else {
    const Node *p = (const Node *)lastEntry;
    Node *p1 =
      (p->succ >= 0)
      ? CALL_I(get, map->nodes, p->succ)
      : next_chain_node(map, hash_index(map, p->entry.key));
    return (p1 == NULL) ? NULL : &p1->entry;
  }
}

static inline const MapEntry *
iterate(const MapI *map, const MapEntry *lastEntry)
{
  return iterate_hash_map((const HashMap *)map, lastEntry);
}

static void
free_hash_map(HashMap *map)
{
  clear_hash_map(map);
  CALL_I(free, map->nodes);
  free(map->hashtable);
  free(map);
}

static inline void
free_map(MapI *map)
{
  free_hash_map((HashMap *)map);
}

static struct MapFns fns = {
  .get = get,
  .put = put,
  .size = size,
  .clear = clear,
  .iterate = iterate,
  .free = free_map
};


MapI *
new_hash_map(CompareFn *compareFn, HashFn *hashFn)
{
  HashMap *map = callocChk(1, sizeof(HashMap));
  map->map.FUNCTIONS_I = &fns;
  map->isCompare = compareFn;
  map->hashFn = hashFn;
  map->nodes = new_dynamic_array(sizeof(Node), NULL);
  rehash(map);
  return &map->map;

}


#ifdef TEST_HASH_MAP

#include "map-unit-test.h"
#include "string-hashcode.h"
#include "unit-test.h"

#include <stdio.h>
#include <string.h>

static int primes[] = {
  2, 3, 5, 7, 11, 13, 17, 19, 23, 29,
  31, 37, 41, 43, 47, 53, 59, 61, 67, 71,
  73, 79, 83, 89, 97, 101, 103, 107, 109, 113,
  127, 131, 137, 139, 149, 151, 157, 163, 167, 173,
  179, 181, 191, 193, 197, 199, 211, 223, 227, 229,
  233, 239, 241, 251, 257, 263, 269, 271, 277, 281,
  283, 293, 307, 311, 313, 317, 331, 337, 347, 349,
  353, 359, 367, 373, 379, 383, 389, 397, 401, 409,
  419, 421, 431, 433, 439, 443, 449, 457, 461, 463,
  467, 479, 487, 491, 499, 503, 509, 521, 523, 541,
  547, 557, 563, 569, 571, 577, 587, 593, 599, 601,
  607, 613, 617, 619, 631, 641, 643, 647, 653, 659,
  661, 673, 677, 683, 691, 701, 709, 719, 727, 733,
  739, 743, 751, 757, 761, 769, 773, 787, 797, 809,
  811, 821, 823, 827, 829, 839, 853, 857, 859, 863,
  877, 881, 883, 887, 907, 911, 919, 929, 937, 941,
  947, 953, 967, 971, 977, 983, 991, 997, 1009, 1013,
  1019, 1021, 1031, 1033, 1039, 1049, 1051, 1061, 1063, 1069,
  1087, 1091, 1093, 1097, 1103, 1109, 1117, 1123, 1129, 1151,
  1153, 1163, 1171, 1181, 1187, 1193, 1201, 1213, 1217, 1223,
  1229, 1231, 1237, 1249, 1259, 1277, 1279, 1283, 1289, 1291,
  1297, 1301, 1303, 1307, 1319, 1321, 1327, 1361, 1367, 1373,
  1381, 1399, 1409, 1423, 1427, 1429, 1433, 1439, 1447, 1451,
  1453, 1459, 1471, 1481, 1483, 1487, 1489, 1493, 1499, 1511,
  1523, 1531, 1543, 1549, 1553, 1559, 1567, 1571, 1579, 1583,
  1597, 1601, 1607, 1609, 1613, 1619, 1621, 1627, 1637, 1657,
  1663, 1667, 1669, 1693, 1697, 1699, 1709, 1721, 1723, 1733,
  1741, 1747, 1753, 1759, 1777, 1783, 1787, 1789, 1801, 1811,
  1823, 1831, 1847, 1861, 1867, 1871, 1873, 1877, 1879, 1889,
  1901, 1907, 1913, 1931, 1933, 1949, 1951, 1973, 1979, 1987,
  1993, 1997, 1999, 2003, 2011, 2017, 2027, 2029, 2039, 2053,
  2063, 2069, 2081, 2083, 2087, 2089, 2099, 2111, 2113, 2129,
  2131, 2137, 2141, 2143, 2153, 2161, 2179, 2203, 2207, 2213,
  2221, 2237, 2239, 2243, 2251, 2267, 2269, 2273, 2281, 2287,
  2293, 2297, 2309, 2311, 2333, 2339, 2341, 2347, 2351, 2357,
  2371, 2377, 2381, 2383, 2389, 2393, 2399, 2411, 2417, 2423,
  2437, 2441, 2447, 2459, 2467, 2473, 2477, 2503, 2521, 2531,
  2539, 2543, 2549, 2551, 2557, 2579, 2591, 2593, 2609, 2617,
  2621, 2633, 2647, 2657, 2659, 2663, 2671, 2677, 2683, 2687,
  2689, 2693, 2699, 2707, 2711, 2713, 2719, 2729, 2731, 2741,
  2749, 2753, 2767, 2777, 2789, 2791, 2797, 2801, 2803, 2819,
};


static void
check_next_primes(void)
{
  for (int i = 1; i < sizeof(primes)/sizeof(primes[0]); i++) {
    int next = next_prime(primes[i - 1]);
    UNIT_TEST(next == primes[i]);
  }
}

static int
string_compare(const void *p1, const void *p2, void *context)
{
  return strcmp(p1, p2);
}


int
main(int argc, const char *argv[])
{
  check_next_primes();
  MapI *map = new_hash_map(string_compare, string_hashcode);
  map_unit_test(map, 0);
  CALL_I(free, map);
  report_unit_test(stderr);
  return (num_fail_unit_test() != 0);
}
#endif //ifdef TEST_HASH_MAP
