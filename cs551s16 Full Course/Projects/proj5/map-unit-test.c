#include "mapi.h"
#include "map-unit-test.h"
#include "unit-test.h"

#include <stdarg.h>
#include <string.h>

static void
test_entries(MapI *map, MapEntry entries[], size_t nEntries, int doTestIterate)
{
  for (int i = 0; i < nEntries; i++) {
    CALL_I(put, map, entries[i].key, entries[i].value);
  }
  UNIT_TEST(CALL_I(size, map) == nEntries);
  for (int i = 0; i < nEntries; i++) {
    const char *value = CALL_I(get, map, entries[i].key);
    UNIT_TEST(strcmp(value, entries[i].value) == 0);
  }
  if (doTestIterate) {
    int i = 0;
    for (const MapEntry *entry = CALL_I(iterate, map, NULL);
         entry != NULL;
         entry = CALL_I(iterate, map, entry)) {
      UNIT_TEST(strcmp(entry->key, entries[nEntries - i - 1].key) == 0);
      UNIT_TEST(strcmp(entry->value, entries[nEntries - i - 1].value) == 0);
      i++;
    }
    UNIT_TEST(i == nEntries);
  }
}

static void
test_multiple(MapI *map, int doTestIterate)
{
  MapEntry entries[] = {
    { "abc", "1" },
    { "def", "2" },
    { "ghi", "3" },
    { "", "4" }
  };
  test_entries(map, entries, sizeof(entries)/sizeof(entries[0]), doTestIterate);
}

static void
test_empty(MapI *map, int doTestIterate)
{
  test_entries(map, NULL, 0, doTestIterate);
}

static void
test_big(MapI *map, int doTestIterate)
{
  enum {
    N_OCTETS = 3,
    N_ENTRIES = 1 << (N_OCTETS * 3)
  };
  typedef struct {
    char key[N_OCTETS + 1];
    char value[N_OCTETS + 1];
  } TestData;
  TestData data[N_ENTRIES];
  MapEntry entries[N_ENTRIES];
  for (int i = 0; i < N_ENTRIES; i++) {
    sprintf(data[i].key, "%0*o", N_OCTETS, i);
    sprintf(data[i].value, "%0*o", N_OCTETS, i);
    entries[i].key = data[i].key; entries[i].value = data[i].value;
  }
  test_entries(map, entries, N_ENTRIES, doTestIterate);
}

void
map_unit_test(MapI *map, int doTestIterate)
{
  CALL_I(clear, map);
  UNIT_TEST(CALL_I(size, map) == 0);
  test_multiple(map, doTestIterate);
  CALL_I(clear, map);
  UNIT_TEST(CALL_I(size, map) == 0);
  test_empty(map, doTestIterate);
  CALL_I(clear, map);
  UNIT_TEST(CALL_I(size, map) == 0);
  test_big(map, doTestIterate);
  CALL_I(clear, map);
  UNIT_TEST(CALL_I(size, map) == 0);
}
