#ifndef _WORD_COUNTS_H
#define _WORD_COUNTS_H

#include <stddef.h>

/** A word counts maintains a map from word strings to word-counts. */

typedef struct WordCountsImpl WordCounts;

typedef struct {
  const char *const word;
  int count;
} WordCount;

/** Return a new word-counts */
WordCounts *new_word_counts(void);

/** Free all resources occupied by counts. */
void free_word_counts(WordCounts *counts);

/** Return count for word in counts; NULL if not present.  It is
 *  permissible to change the count (but not the word) in the return'd
 *  word-count which will update in the counts.
 */
WordCount *get_word_counts(const WordCounts *counts, const char *word);

/** Add word to counts (increment count if already present). Return
 *  updated word-count.  It is permissible to change the count (but
 *  not the word) in the return'd word-count which will update in the
 *  counts.
 */
WordCount *add_word_counts(WordCounts *counts, const char *word);

/** Return number of words in counts. */
size_t size_word_counts(const WordCounts *counts);

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
WordCount *index_word_counts(const WordCounts *counts, int index);

#endif //ifndef _WORD_COUNTS_H
