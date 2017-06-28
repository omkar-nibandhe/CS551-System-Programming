#include "data-buffer.h"
#include "dynamic-array.h"
#include "errors.h"
#include "file-words.h"
#include "memalloc.h"
#include "word-char.h"
#include "word-counts.h"

#include <assert.h>
#include <errno.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/** Keeps track of state in output of "word count\n" lines. */
struct State {
  enum { //listed in order of transitions
    INIT_STATE,    /** at start of word-count pair:
                    *  index has # of word-count's completed so far
                    */
    WORD_STATE,    /** in word: nWordChars has # of word chars output so far */
    SPACE_STATE,   /** between word and count:
                    *  nSpaceChars has # of spaces output
                    */
    COUNT_STATE,   /** in count followed by newline:
                    * nCountChars has # of count char's output
                    */
    END_STATE      /** completed all word-count pairs */
  } state;
  int index;       /** index is index of current word-count pair */
  int nWordChars;  /** # of word chars output so far */
  int nSpaceChars; /** # of spaces output so far */
  int nCountChars; /** # of count chars output so far */
};

struct Worker {
  WordCounts *stops;   /** stop words from stop file */
  WordCounts *counts;  /** counts for argument text files */
  ArrayI *maxWords;    /** the words to be reported */
  struct State state;  /** tracks output state */
};

static WordCounts *
get_stop_counts(FILE *f, const char *fileName)
{
  WordCounts *stopCounts = new_word_counts();
  append_word_counts(stopCounts, f, fileName, stopCounts);
  return stopCounts;
}

static WordCounts *
get_text_counts(WordCounts *stops, FILE *files[],
                const char *fileNames[], int nFiles)
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


static struct Worker *
make_worker(int maxNWords, FILE *stopFile, const char *stopFileName,
            FILE *textFiles[], const char *textFileNames[], int nTextFiles)
{
  struct Worker *worker = mallocChk(sizeof(struct Worker));
  worker->stops = get_stop_counts(stopFile, stopFileName);
  worker->counts = get_text_counts(worker->stops, textFiles,
                                   textFileNames, nTextFiles);
  worker->maxWords = get_max_words(maxNWords, worker->counts);
  int nWords = CALL_I(size, worker->maxWords);
  struct State *sP = &worker->state;
  sP->state = (nWords > 0) ? INIT_STATE : END_STATE;
  sP->index = sP->nWordChars = sP->nSpaceChars = sP->nCountChars = 0;
  return worker;
}

static int
string_to_non_negative_int(const char *string, const char *name,
                           struct DataBuffer *error)
{
  char *p;
  int n = (int)strtol(string, &p, 10);
  if (p == string || *p != '\0' || n < 0) {
    error->count =
      snprintf(error->data, error->size,
               "cannot convert \"%s\" to non-negative int for %s\n",
               string, name);
    error->isError = true;
    return -1;
  }
  return n;
}

/** Do work for request.  request is assumed to contain the following
 *  fields with each field terminated by a single NUL '\0' character:
 *
 *   nArgs:      # of arguments which follow (not including itself).
 *   maxNWords:  max number of words to be output.
 *   fileNames:  file-names (as many as left-over from nArgs value).
 *
 * If an error is detected, the error is written to error and a NULL
 * is returned.
 */
struct Worker *
new_worker(const char *request, struct DataBuffer *error)
{
  error->count = 0;
  error->isError = false;
  const char *nArgsString = request;
  const char *maxNWordsString = nArgsString + strlen(nArgsString) + 1;
  const char *fileNameStrings = maxNWordsString + strlen(maxNWordsString) + 1;
  int nArgs = string_to_non_negative_int(nArgsString, "N_ARGS", error);
  if (nArgs < 0) return NULL;
  int maxNWords =
    string_to_non_negative_int(maxNWordsString, "MAX_N_WORDS", error);
  if (maxNWords < 0) return NULL;
  int nFileNames = nArgs - 1;
  const char *fileNames[nFileNames];
  FILE *files[nFileNames];
  const char *currentFileName = fileNameStrings;
  for (int i = 0; i < nFileNames; i++) {
    fileNames[i] = currentFileName;
    if ((files[i] = fopen(currentFileName, "r")) == NULL) {
      error->isError = true;
      error->count =
        snprintf(error->data, error->size - error->count,
                 "cannot read file %s: %s\n",
                 currentFileName, strerror(errno));
      return NULL;
    }
    currentFileName += strlen(currentFileName) + 1;
  }
  struct Worker *worker = make_worker(maxNWords, files[0], fileNames[0],
                                      &files[1], &fileNames[1], nFileNames - 1);
  for (int i = 0; i < nFileNames; i++) {
    fclose(files[i]);
  }
  return worker;
}

void
free_worker(struct Worker *worker)
{
  CALL_I(free, worker->maxWords);
  free_word_counts(worker->counts);
  free_word_counts(worker->stops);
  free(worker);
}

/** Write chars of response from worker into dataBuffer at start of
 *  buffer.  Returns # of chars written.  Returns 0 if no more
 *  response.
 */
size_t
next_response(struct Worker *worker, struct DataBuffer *dataP)
{
  struct State *stateP = &worker->state;
  dataP->count = 0;
  if (stateP->state == END_STATE) return 0;
  bool isDone = false;
  do { //while (!isDone)
    WordCount *wc = CALL_I(get, worker->maxWords, stateP->index);
    switch (stateP->state) {
    case INIT_STATE:
      stateP->nWordChars = stateP->nSpaceChars = stateP->nCountChars = 0;
      stateP->state = WORD_STATE;
      break;
    case WORD_STATE: {
      int nToCopy = strlen(wc->word) - stateP->nWordChars;
      assert(nToCopy > 0);
      int n = append_chars_to_data_buffer(&wc->word[stateP->nWordChars],
                                          nToCopy, dataP);
      assert(n > 0);
      stateP->nWordChars += n;
      if (n == nToCopy) {
        stateP->state = SPACE_STATE;
      }
      else {
        isDone = true;
      }
      break;
    }
    case SPACE_STATE: {
      assert(stateP->nSpaceChars == 0);
      stateP->nSpaceChars = append_chars_to_data_buffer(" ", 1, dataP);
      if (stateP->nSpaceChars != 0) {
        stateP->state = COUNT_STATE;
      }
      else {
        isDone = true;
      }
      break;
    }
    case COUNT_STATE: {
      enum { MAX_COUNT_CHARS = 16 };  //cleaner to determine dynamically.
      char countBuffer[MAX_COUNT_CHARS];
      int countLen = snprintf(countBuffer, MAX_COUNT_CHARS, "%d\n", wc->count);
      assert(countLen < MAX_COUNT_CHARS);
      size_t nToCopy = countLen - stateP->nCountChars;
      assert(nToCopy > 0);
      int n = append_chars_to_data_buffer(&countBuffer[stateP->nCountChars],
                                          nToCopy, dataP);
      assert(n > 0);
      stateP->nCountChars += n;
      if (n == nToCopy) {
        stateP->index++;
        int nWords = CALL_I(size, worker->maxWords);
        stateP->state = (stateP->index >= nWords) ? END_STATE : INIT_STATE;
      }
      else {
        isDone = true;
      }
      break;
    }
    case END_STATE:
      isDone = true;
      break;
    default:
      assert(false);
    }
  } while (!isDone);
  assert(dataP->count > 0);
  return dataP->count;
}


#ifdef TEST_WORKER

#include <math.h>

//Interactive test routine: args as per usage message
int
main(int argc, const char *argv[])
{
  if (argc < 4) {
    fprintf(stderr, "usage: %s MAX_N_WORDS STOP_FILE_NAME TEXT_FILE_NAME...\n",
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
  enum { DATA_BUFFER_SIZE = 128 };
  struct DataBuffer *dataBuffer = new_data_buffer(DATA_BUFFER_SIZE);
  struct Worker *worker = new_worker(request, dataBuffer);
  if (worker == NULL) {
    fprintf(stderr, "%.*s", (int)dataBuffer->count, dataBuffer->data);
  }
  int n;
  while ((n = next_response(worker, dataBuffer)) != 0) {
    printf("%.*s", n, dataBuffer->data);
  }
  free_worker(worker);
  free_data_buffer(dataBuffer);
  return 0;
}
#endif
