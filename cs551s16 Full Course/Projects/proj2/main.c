#include "dynamic-array.h"
#include "errors.h"
#include "file-words.h"
#include "realloc-buf.h"
#include "string-reader.h"
#include "word-counts.h"

#include <ctype.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <fcntl.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

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

/** Redirect stdout for child to fds[1] */
static void
redirect_stdout(int fds[2])
{
  close(fds[0]);
  if (fds[1] != STDOUT_FILENO) {
    if (dup2(fds[1], STDOUT_FILENO) < 0) {
      exit(1);
    }
    close(fds[1]);
  }
}

static int
do_wait(pid_t pids[], int nProcesses)
{
  int hasError = 0;
  for (int i = 0; i < nProcesses; i++) {
    int status;
    if (waitpid(pids[i], &status, 0) < 0) {
      fprintf(stderr, "cannot wait for child %d: %s\n", i, strerror(errno));
      hasError = 1;
    }
    if (status != 0) {
      fprintf(stderr, "error in child %d: %s\n", i, strerror(errno));
      hasError = 1;
    }
  }
  return hasError;
}

static void
do_child(WordCounts *stops, FILE *f, const char *fName)
{
  WordCounts *counts = new_word_counts();
  append_word_counts(stops, f, fName, counts);
  int nWords = size_word_counts(counts);
  for (int i = 0; i < nWords; i++) {
    WordCount *wc = index_word_counts(counts, i);
    fprintf(stdout, "%s %d\n", wc->word, wc->count);
  }
  free_word_counts(counts);
}

static int
is_non_space(int c)
{
  return !isspace(c);
}

static WordCounts *
do_parent(FILE *inFiles[], int nFiles)
{
  WordCounts *counts = new_word_counts();
  ReallocBuf *buf = new_realloc_buf();
  for (int i = 0; i < nFiles; i++) {
    for (char *word = read_string(inFiles[i], "", buf, is_non_space);
         word != NULL;
         word = read_string(inFiles[i], "", buf, is_non_space)) {
      WordCount *wc = add_word_counts(counts, word);
      char *count = read_string(inFiles[i], "", buf, is_non_space);
      wc->count += atoi(count) - 1;
    }
    fclose(inFiles[i]);
  }
  free_realloc_buf(buf);
  return counts;
}

static void
go(int maxNWords, WordCounts *stops,
   FILE *textFiles[], const char *textFileNames[], int nTextFiles)
{
  FILE *inFiles[nTextFiles]; //files for pipe read-ends for parent
  pid_t pids[nTextFiles];    //child pids
  int hasError = 0;
  int i;
  for (i = 0; !hasError && i < nTextFiles; i++) {
    int fds[2];
    if (pipe(fds) < 0) {
      fprintf(stderr, "cannot created pipe for file %s: %s\n",
              textFileNames[i], strerror(errno));
      hasError = 1;
      break;
    }
    if ((pids[i] = fork()) < 0) {
      fprintf(stderr, "cannot fork child for file %s: %s\n",
              textFileNames[i], strerror(errno));
      hasError = 1;
    }
    else if (pids[i] == 0) { //child
      redirect_stdout(fds);
      for (int j = 0; j < nTextFiles; j++) {
        if (i != j) {
          FILE *toClose = (i < j) ? textFiles[j] : inFiles[j];
          if (fclose(toClose) != 0) {
            exit(1);
          }
        }
      }
      do_child(stops, textFiles[i], textFileNames[i]);
      fclose(textFiles[i]);
      free_word_counts(stops);
      exit(0);
    }
    else { //parent
      fclose(textFiles[i]);
      close(fds[1]);
      if ((inFiles[i] = fdopen(fds[0], "r")) == NULL) {
        fprintf(stderr, "cannot fdopen pipe read end for %d: %s\n",
                i, strerror(errno));
        hasError = 1;
      }
    }
  } //for: have forked children
  WordCounts *counts = do_parent(inFiles, i);
  hasError = do_wait(pids, i) || hasError;
  if (hasError) {
    fatal("error processing children");
  }
  else {
    ArrayI *maxWords = get_max_words(maxNWords, counts);
    report_max_words(maxWords, stdout);
    CALL_I(free, maxWords);
  }
  free_word_counts(counts);
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
  WordCounts *stops = get_stop_counts(files[0], argv[2]);
  fclose(files[0]);
  go(n, stops, &files[1], &argv[3], argc - 3);
  free_word_counts(stops);
  return 0;
}
