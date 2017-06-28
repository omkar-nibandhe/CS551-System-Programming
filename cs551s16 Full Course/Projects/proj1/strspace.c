#include "memalloc.h"
#include "strspace.h"
#include "string-reader.h"

#include "stdlib.h"
#include "string.h"

/** Provide permanent storage for intern'd strings */

typedef struct ChunkStruc {
  struct ChunkStruc *succ; /** next chunk in strspace */
  char chunk[];
} Chunk;

struct StrSpaceImpl {
  size_t size;     /** size of current chunk */
  size_t index;    /** index of next free char in current chunk */
  Chunk *chunks;   /** list of chunks allocated to this strspace */
};

/** Create a new string-space */
StrSpace *
new_strspace(void)
{
  return callocChk(1, sizeof(StrSpace));
}

/** Free all resources used by strspace. */
void free_strspace(StrSpace *strspace) {
  Chunk *last;
  for (Chunk *p = strspace->chunks; p != NULL; p = last) {
    last = p->succ;
    free(p);
  }
  free(strspace);
}

enum { NOMINAL_CHUNK_SIZE = 4088 };

/** Intern NUL-terminated string in strspace and return pointer to
 *  intern'd value.
 */
const char *
intern_strspace(StrSpace *strspace, const char *string)
{
  int len = strlen(string);
  int n = len + 1;
  if (strspace->size - strspace->index < n) {
    int chunkSize = (n < NOMINAL_CHUNK_SIZE) ? NOMINAL_CHUNK_SIZE : n;
    Chunk *chunk = mallocChk(sizeof(Chunk) + chunkSize);
    chunk->succ = strspace->chunks;
    strspace->chunks = chunk;
    strspace->size = chunkSize;
    strspace->index = 0;
  }
  char *dest = strspace->chunks->chunk + strspace->index;
  strcpy(dest, string);
  strspace->index += n;
  return dest;
}


#ifdef TEST_STRSPACE

#include "unit-test.h"

int
main(int argc, const char *argv[])
{
  enum { BIG_LEN = 2*NOMINAL_CHUNK_SIZE + 2 };
  char big[BIG_LEN + 1];
  memset(big, 'a', BIG_LEN);
  big[BIG_LEN] = '\0';
  const char *const tests[] = {
    "", "a", "012345678901234567890123456789012345678901234567890123456789",
    "a\nbcd\t\t\f", big };
  const int nTests = sizeof(tests)/sizeof(tests[0]);
  enum { N_LOOPS = 5 };
  const char *interned[N_LOOPS * nTests];
  int internedIndex = 0;
  StrSpace *strspace = new_strspace();
  for (int i = 0; i < N_LOOPS; i++) {
    for (int j = 0; j < nTests; j++) {
      interned[internedIndex++] = intern_strspace(strspace, tests[j]);
    }
  }
  internedIndex = 0;
  for (int i = 0; i < N_LOOPS; i++) {
    for (int j = 0; j < nTests; j++) {
      UNIT_TEST(strcmp(tests[j], interned[internedIndex++]) == 0);
    }
  }
  free_strspace(strspace);
  report_unit_test(stderr);
  return (num_fail_unit_test() != 0);
}

#endif //ifdef TEST_STRSPACE
