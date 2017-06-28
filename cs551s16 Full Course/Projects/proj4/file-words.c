#include "file-words.h"
#include "realloc-buf.h"
#include "string-reader.h"
#include "word-counts.h"

#include <ctype.h>
#include <stddef.h>
#include <stdio.h>

static inline int
is_word_char(int c)
{
  return isalnum(c) || c == '\'';
}

/** Destructively modifies string to lower-case */
static char *
to_lower(char *string)
{
  for (char *p = string; *p != '\0'; p++) {
    *p = tolower(*p);
  }
  return string;
}

/** Append words read from file f with name fileName to counts. */
void
append_word_counts(WordCounts *stopCounts,
                   FILE *f, const char *fileName, WordCounts *counts)
{
  append_word_counts_f(is_word_char, stopCounts, f, fileName, counts);
}

/** Append words (as defined by isWordChar) read from file f with name
 *  fileName to counts as long as stopCounts does not contain word.
 */
void append_word_counts_f(is_word_char_f *isWordChar, WordCounts *stopCounts,
                          FILE *f, const char *fileName, WordCounts *counts)
{
  ReallocBuf *buf = new_realloc_buf();
  for (char *word = read_string(f, fileName, buf, is_word_char);
       word != NULL;
       word = read_string(f, fileName, buf, isWordChar)) {
    word = to_lower(word);
    if (get_word_counts(stopCounts, word) == 0) {
      add_word_counts(counts, to_lower(word));
    }
  }
  free_realloc_buf(buf);
}

#ifdef TEST_FILE_WORDS

#include "errors.h"
#include "unit-test.h"

#include <stdlib.h>
#include <string.h>

static void
do_test(const char *contents, const char *fileName,
        const WordCount words[], int nWords)
{
  FILE *f =
    (strlen(contents) == 0)
    ? fopen("/dev/null", "r")
    : fmemopen((char *)contents, strlen(contents), "r");
  if (!f) {
    fatal("cannot open file %s with contents \"%s\":", fileName, contents);
  }
  WordCounts *stops = new_word_counts();
  WordCounts *counts = new_word_counts();
  append_word_counts(stops, f, fileName, counts);
  UNIT_TEST(size_word_counts(counts) == nWords);
  for (int i = 0; i < nWords; i++) {
    WordCount *wc = get_word_counts(counts, words[i].word);
    UNIT_TEST(wc->count == words[i].count);
  }
  free_word_counts(counts);
  free_word_counts(stops);
  if (fclose(f) != 0){
    fatal("cannot close %s:", fileName);
  }
}

static void
test_empty(void)
{
  do_test("", "empty.test", NULL, 0);
}

static void
test_non_words(void)
{
  do_test("  #\n$$", "non-words.test", NULL, 0);
}

static void
test_mixed_case(void)
{
  const char *buf = "abc ABC#aBC isn't ISn't";
  WordCount wordCounts[] = {
    { .word = "abc", .count = 3 },
    { .word = "isn't", .count = 2 },
  };
  const int nWords = sizeof(wordCounts)/sizeof(wordCounts[0]);
  do_test(buf, "mixed-case.test", wordCounts, nWords);
}

static void
test_content(void)
{
  WordCount words[] = {
    { .word = "012345", .count = 0 },
    { .word = "6", .count = 0 },
    { .word = "7", .count = 0 },
    { .word = "8", .count = 0 },
    { .word = "88", .count = 0 },
    { .word = "9", .count = 0 },
    { .word = "abc", .count = 0 },
    { .word = "def", .count = 0 },
    { .word = "ghi", .count = 0 },
    { .word = "isn't", .count = 0 },
    { .word = "jkl", .count = 0 },
    { .word = "mno", .count = 0 },
    { .word = "p1'", .count = 0 },
    { .word = "pqr", .count = 0 },
    { .word = "stuv", .count = 0 },
    { .word = "wxy", .count = 0 },
    { .word = "z", .count = 0 },
  };
  const int nWords = sizeof(words)/sizeof(words[0]);
  const char *interWords[] = {
    " ", "\n", "\t", " \t", "#", "$%",
  };
  const int nInterWords = sizeof(interWords)/sizeof(interWords[0]);
  enum { N_TOTAL_WORDS = 1000 };
  ReallocBuf *buf = new_realloc_buf();
  for (int i = 0; i < N_TOTAL_WORDS; i++) {
    int wordIndex = rand() % nWords;
    const char *word = words[wordIndex].word;
    append_realloc_buf(buf, word, strlen(word));
    words[wordIndex].count++;
    const char *interWord = interWords[rand() % nInterWords];
    append_realloc_buf(buf, interWord, strlen(interWord));
  }
  const char NUL = '\0';
  append_realloc_buf(buf, &NUL, 1);
  do_test(get_realloc_buf(buf), "content.test", words, nWords);
  free_realloc_buf(buf);
}

int
main(int argc, const char *argv[])
{
  test_empty();
  test_non_words();
  test_mixed_case();
  test_content();
  report_unit_test(stderr);
  return (num_fail_unit_test() != 0);
}


#endif //ifdef TEST_FILE_WORDS
