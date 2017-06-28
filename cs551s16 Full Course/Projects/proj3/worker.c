#include "dynamic-array.h"
#include "errors.h"
#include "file-words.h"
#include "word-char.h"
#include "word-counts.h"

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <dlfcn.h>

static WordCounts *
get_stop_counts(is_word_char_f *word_char_f, FILE *f, const char *fileName)
{
  WordCounts *stopCounts = new_word_counts();
  append_word_counts_f(word_char_f, stopCounts, f, fileName, stopCounts);
  return stopCounts;
}

static WordCounts *
get_text_counts(is_word_char_f *word_char_f, WordCounts *stops,
                FILE *files[], const char *fileNames[], int nFiles)
{
  WordCounts *textCounts = new_word_counts();
  for (int i = 0; i < nFiles; i++) {
    append_word_counts_f(word_char_f, stops,
                         files[i], fileNames[i], textCounts);
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

#ifdef TEST_WORKER
#define REPORT_DELIM '\n'
#else
#define REPORT_DELIM '\0'
#endif

static void
report_max_words(const ArrayI *maxWords, FILE *out)
{
  int nWords = CALL_I(size, maxWords);
  for (int i = 0; i < nWords; i++) {
    WordCount *wc = CALL_I(get, maxWords, i);
    fprintf(out, "%d %s%c", wc->count, wc->word, REPORT_DELIM);
  }
}

static void
go(is_word_char_f *word_char_f, int maxNWords,
   FILE *stopFile, const char *stopFileName,
   FILE *textFiles[], const char *textFileNames[], int nTextFiles, FILE *out)
{
  WordCounts *stops = get_stop_counts(word_char_f, stopFile, stopFileName);
  WordCounts *counts =
    get_text_counts(word_char_f, stops, textFiles, textFileNames, nTextFiles);
  ArrayI *maxWords = get_max_words(maxNWords, counts);
  report_max_words(maxWords, out);
  CALL_I(free, maxWords);
  free_word_counts(counts);
  free_word_counts(stops);
}

static void *
open_dll(const char *modName, FILE *err)
{
  void *dlHandle = dlopen(modName, RTLD_NOW);
  if (dlHandle == NULL) {
    fprintf(err, "-1 cannot open dynamic module %s: %s%c",
            modName, dlerror(), REPORT_DELIM);
    exit(1);
  }
  return dlHandle;
}

static void *
get_dll_sym(void *dllHandle, const char *modName, const char *symbol, FILE *err)
{
  void *symbolValue = dlsym(dllHandle, symbol);
  if (symbolValue == NULL) {
    fprintf(err, "-2 cannot resolve %s in %s: %s%c",
            symbol, modName, dlerror(), REPORT_DELIM);
    exit(1);
  }
  return symbolValue;
}

static void
close_dll(void *dllHandle, const char *modName, FILE *err)
{
  if (dlclose(dllHandle) != 0) {
    fprintf(err, "-3 cannot close module %s: %s%c",
            modName, dlerror(), REPORT_DELIM);
    exit(1);
  }
}

static int
string_to_non_negative_int(const char *string, const char *name, FILE *err)
{
  char *p;
  int n = (int)strtol(string, &p, 10);
  if (p == string || *p != '\0' || n < 0) {
    fprintf(err, "cannot convert \"%s\" to non-negative int for %s%c",
            string, name, REPORT_DELIM);
    exit(1);
  }
  return n;
}

/** Do work for request.  request is assumed to contain the following
 *  fields with each field terminated by a single NUL '\0' character:
 *
 *   nArgs:      # of arguments which follow (not including itself).
 *   moduleName: name of the module containing is-word-char definition.
 *   maxNWords:  max number of words to be output.
 *   fileNames:  file-names (as many as left-over from nArgs value).
 *
 * Up to maxNWords are written to out in the following format:
 *    A word count
 *    A single space
 *    A NUL-terminated word.
 * If an error is detected, the error is written to out preceeded
 * by a negative count.
 */
void
do_work(const char *request, FILE *out)
{
  const char *nArgsString = request;
  const char *moduleName = nArgsString + strlen(nArgsString) + 1;
  const char *maxNWordsString = moduleName + strlen(moduleName) + 1;
  const char *fileNameStrings = maxNWordsString + strlen(maxNWordsString) + 1;
  int nArgs = string_to_non_negative_int(nArgsString, "N_ARGS", out);
  void *dllHandle = open_dll(moduleName, out);
  is_word_char_f *word_char_f =
    (is_word_char_f *)get_dll_sym(dllHandle, moduleName,
                                  IS_WORD_CHAR_F_NAME, out);
  int maxNWords =
    string_to_non_negative_int(maxNWordsString, "MAX_N_WORDS", out);
  int nFileNames = nArgs - 2;
  const char *fileNames[nFileNames];
  FILE *files[nFileNames];
  const char *currentFileName = fileNameStrings;
  for (int i = 0; i < nFileNames; i++) {
    fileNames[i] = currentFileName;
    if ((files[i] = fopen(currentFileName, "r")) == NULL) {
      fprintf(out, "-1 cannot read file %s: %s%c", currentFileName,
              strerror(errno), REPORT_DELIM);
      exit(1);
    }
    currentFileName += strlen(currentFileName) + 1;
  }
  go(word_char_f, maxNWords, files[0], fileNames[0],
     &files[1], &fileNames[1], nFileNames - 1, out);
  for (int i = 0; i < nFileNames; i++) {
    fclose(files[i]);
  }
  close_dll(dllHandle, moduleName, out);
}

#ifdef TEST_WORKER

#include <math.h>

//Interactive test routine: args as per usage message
int
main(int argc, const char *argv[])
{
  if (argc < 5) {
    fprintf(stderr, "usage: %s MODULE_NAME MAX_N_WORDS "
            "STOP_FILE_NAME TEXT_FILE_NAME...\n",
            argv[0]);
    exit(1);
  }
  size_t requestSize = 0;
  for (int i = 1; i < argc; i++) { //accumulate size of all args
    requestSize += strlen(argv[i]) + 1;
  }
  int nArgs = argc - 1;
  int nArgsDigits = log10(nArgs) + 1;
  char request[nArgsDigits + 1 + requestSize];
  sprintf(request, "%d", nArgs);
  char *p = request + strlen(request) + 1;
  for (int i = 1; i < argc; i++) { //copy all args into request
    strcpy(p, argv[i]);
    p += strlen(argv[i]) + 1;
  }
  do_work(request, stdout);
  return 0;
}
#endif
