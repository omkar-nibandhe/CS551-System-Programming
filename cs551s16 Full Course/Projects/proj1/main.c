#include "dynamic-array.h"
#include "errors.h"
#include "file-words.h"
#include "word-counts.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static void
usage(const char *program)
{
  fprintf(stderr, "usage: %s MAX_N_WORDS STOP_FILE_NAME TEXT_FILE_NAME...\n",
          program);
  exit(1);
}

static WordCounts *
get_stop_counts(FILE *f, const char *fileName)
{
  WordCounts *stopCounts = new_word_counts();
  append_word_counts(stopCounts, f, fileName, stopCounts);
  return stopCounts;
}

static WordCounts *
get_text_counts(WordCounts *stops,
                FILE *files[], const char *fileNames[], int nFiles)
{
  WordCounts *textCounts = new_word_counts();
  for (int i = 0; i < nFiles; i++) {
    append_word_counts(stops, files[i], fileNames[i], textCounts);
  }
  return textCounts;
}

static int
compare_word_count(const void *p1, const void *p2, void *context)
{
  const WordCount *wc1 = (const WordCount *)p1;
  const WordCount *wc2 = (const WordCount *)p2;
  int n = wc2->count - wc1->count;
  return (n == 0) ? strcmp(wc2->word, wc1->word) : n;
}

static ArrayI *
get_max_words(int maxNWords, const WordCounts *counts)
{
  ArrayOptions options = {
    .maxNElements = maxNWords,
    .compareFn = compare_word_count
  };
  ArrayI *maxWords = new_dynamic_array(sizeof(WordCount), &options);
  int nWords = size_word_counts(counts);
  for (int i = 0; i < nWords; i++) {
    WordCount *wc = index_word_counts(counts, i);
    CALL_I(add, maxWords, wc);
  }
  return maxWords;
}

static void
report_max_words(const ArrayI *maxWords, FILE *out)
{
  int nWords = CALL_I(size, maxWords);
  for (int i = 0; i < nWords; i++) {
    WordCount *wc = CALL_I(get, maxWords, i);
    fprintf(out, "%s %d\n", wc->word, wc->count);
  }
}

static void
go(int maxNWords, FILE *stopFile, const char *stopFileName,
   FILE *textFiles[], const char *textFileNames[], int nTextFiles)
{
  WordCounts *stops = get_stop_counts(stopFile, stopFileName);
  WordCounts *counts =
    get_text_counts(stops, textFiles, textFileNames, nTextFiles);
  ArrayI *maxWords = get_max_words(maxNWords, counts);
  report_max_words(maxWords, stdout);
  CALL_I(free, maxWords);
  free_word_counts(counts);
  free_word_counts(stops);
}

int
main(int argc, const char *argv[])
{
  if (argc < 4) usage(argv[0]);
  char *p;
  int n = (int)strtol(argv[1], &p, 10);
  if (p == argv[1] || *p != '\0' || n < 0) usage(argv[0]);
  FILE *files[argc - 2];
  for (int i = 2; i < argc; i++) {
    if ((files[i - 2] = fopen(argv[i], "r")) == NULL) {
      error("cannot read file %s:", argv[i]);
    }
  }
  if (getErrorCount() > 0) exit(1);
  go(n, files[0], argv[2], &files[1], &argv[3], argc - 3);
  for (int i = 2; i < argc; i++) {
    fclose(files[i - 2]);
  }
  return 0;
}
