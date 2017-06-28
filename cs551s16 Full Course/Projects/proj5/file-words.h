#ifndef _FILE_WORDS_H
#define _FILE_WORDS_H

#include "word-char.h"
#include "word-counts.h"

#include <stdio.h>

/** Append words read from file f with name fileName to counts
 *  as long as stopCounts does not contain word.
 */
void append_word_counts(WordCounts *stopCounts, FILE *f,
                        const char *fileName, WordCounts *counts);

/** Append words (as defined by isWordChar) read from file f with name
 *  fileName to counts as long as stopCounts does not contain word.
 */
void append_word_counts_f(is_word_char_f *isWordChar, WordCounts *stopCounts,
                          FILE *f, const char *fileName, WordCounts *counts);

#endif //ifndef _FILE_WORDS_H
