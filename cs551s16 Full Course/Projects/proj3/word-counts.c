#include "dynamic-array.h"
#include "memalloc.h"
#include "strspace.h"
#include "word-counts.h"

#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#define HASH_MAP_TYPE 0
#define LINKED_LIST_MAP_TYPE 1

#ifndef MAP_TYPE
#define MAP_TYPE HASH_MAP_TYPE
#endif

#if MAP_TYPE == LINKED_LIST_MAP_TYPE
#include "linked-list-map.h"
#define MAP_CONS new_linked_list_map((CompareFn*)string_compare)
#endif

#if MAP_TYPE == HASH_MAP_TYPE
#include "hash-map.h"
#include "string-hashcode.h"

#define MAP_CONS new_hash_map((CompareFn*)string_compare, string_hashcode)
#endif

/** Word-counts maintains a map from word strings to word-counts. */

struct WordCountsImpl {
  ArrayI *wordCounts;  /** storage for WordCount structs */
  MapI *map;           /** maps words into index into wordCounts */
  StrSpace *strspace;  /** storage for chars in words */
};

/** Return 0 if key1 == key2, >0 if key1 > key2, <0 if key1 < key2 */
static int
string_compare(MapKey key1, MapKey key2, void *context)
{
  const char *s1 = key1;
  const char *s2 = key2;
  return strcmp(s1, s2);
}

// We store array indexes as pointer converted using following routines.
// Note that intptr_t is optional in <stdint.h>; if not present, maybe
// long may work.
static inline void *index_to_ptr(int i) { return (void *)(intptr_t)(i + 1); }

static inline int ptr_to_index(void *p) { return ((int)(intptr_t)p) - 1; }


/** Return a new word-counts */
WordCounts *
new_word_counts(void)
{
  WordCounts *counts = mallocChk(sizeof(WordCounts));
  counts->wordCounts = new_dynamic_array(sizeof(WordCount), NULL);
  counts->map = MAP_CONS;
  counts->strspace = new_strspace();
  return counts;
}

/** Free all resources occupied by counts. */
void
free_word_counts(WordCounts *counts)
{
  CALL_I(free, counts->wordCounts);
  CALL_I(free, counts->map);
  free_strspace(counts->strspace);
  free(counts);
}

/** Return count for word in counts; NULL if not present.  It is
 *  permissible to change the count (but not the word) in the return'd
 *  word-count which will update in the counts.
 */
WordCount *
get_word_counts(const WordCounts *counts, const char *word)
{
  void *p = CALL_I(get, counts->map, (MapKey)word);
  if (p == NULL) {
    return NULL;
  }
  else {
    int index = ptr_to_index(p);
    return CALL_I(get, counts->wordCounts, index);
  }
}

/** Add word to counts (increment count if already present). Return
 *  updated word-count.  It is permissible to change the count (but
 *  not the word) in the return'd word-count which will update in the
 *  counts.
 */
WordCount *
add_word_counts(WordCounts *counts, const char *word)
{
  WordCount *wc = NULL;
  void *p = CALL_I(get, counts->map, (MapKey)word);
  if (p != NULL) {
    int index = ptr_to_index(p);
    wc = CALL_I(get, counts->wordCounts, index);
  }
  else {
    const char *newWord = intern_strspace(counts->strspace, word);
    WordCount wc0 = { .word = newWord, .count = 0 };
    int index = CALL_I(add, counts->wordCounts, &wc0);
    wc = CALL_I(get, counts->wordCounts, index);
    CALL_I(put, counts->map, (MapKey)newWord, (MapValue)index_to_ptr(index));
  }
  wc->count++;
  return wc;
}

/** Return number of words in counts. */
size_t
size_word_counts(const WordCounts *counts)
{
  return CALL_I(size, counts->wordCounts);
}

/** Return WordCount at counts[index].  It is
 *  permissible to change the count (but not the word) in the return'd
 *  word-count which will update in the counts.
 *
 *  Note that it is possible to iterate over a word-counts counts as
 *  follows:
 *
 *  for (int i = 0; i < size_word_counts(counts); i++) {
 *    WordCount *wc = index_word_counts(counts, i);
 *    //process *wc as desired
 *  }
 */
WordCount *
index_word_counts(const WordCounts *counts, int index)
{
  return CALL_I(get, counts->wordCounts, index);
}

#ifdef TEST_WORD_COUNTS

#include "unit-test.h"

int
main(int argc, const char *argv[])
{
  const char *words[] = {
    "abc", "def", "ghi", "jkl", "mno", "pqr", "stu", "vwx", "yz",
    "ABC", "DEF", "GHI", "JKL", "MNO", "PQR", "STU", "VWX", "YZ",
    "012", "345", "678", "90", "!@#", "$%^", "&*(", ")_+", "-=",
    "[]\\", "{}|", ";'\"", ":,.", "<>?", " \n\t",
  };
  size_t nWords = sizeof(words)/sizeof(words[0]);
  WordCounts *counts = new_word_counts();
  UNIT_TEST(size_word_counts(counts) == 0);
  for (int i = 0; i < nWords; i++) {
    for (int j = i + 1; j < nWords; j++) {
      add_word_counts(counts, words[i]);
    }
    add_word_counts(counts, words[i]);
    WordCount *wc = get_word_counts(counts, words[i]);
    UNIT_TEST(wc->count == nWords - i);
    UNIT_TEST(size_word_counts(counts) == i + 1);
  }
  UNIT_TEST(size_word_counts(counts) == nWords);
  for (int i = 0; i < nWords; i++) {
    WordCount *wc = index_word_counts(counts, i);
    UNIT_TEST(strcmp(wc->word, words[i]) == 0);
    UNIT_TEST(wc->count == nWords - i);
  }
  report_unit_test(stderr);
  return (num_fail_unit_test() != 0);
}

#endif //ifdef TEST_WORD_COUNTS
