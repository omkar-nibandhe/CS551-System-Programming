#include "memalloc.h"
#include "realloc-buf.h"

#include <stddef.h>
#include <stdlib.h>
#include <string.h>

/** A realloc'd buffer which is double'd in size whenever it is too small.
 */

struct ReallocBufImpl {
  size_t size;    /** currently allocated size */
  size_t index;   /** index of next free location in buf */
  char *buf;      /** realloc'd buffer */
};


/** Return a new realloc-buf with initSize */
ReallocBuf *
new_sized_realloc_buf(size_t initSize)
{
  ReallocBuf *bufP = mallocChk(sizeof(ReallocBuf));
  bufP->buf = mallocChk(initSize);
  bufP->size = initSize;
  bufP->index = 0;
  return bufP;
}

/** Return a new realloc-buf. */
ReallocBuf *
new_realloc_buf(void)
{
  enum { INIT_SIZE = 64 };
  return new_sized_realloc_buf(INIT_SIZE);
}

/** Free all resources used by buf */
void
free_realloc_buf(ReallocBuf *buf)
{
  free(buf->buf);
  free(buf);
}

/** Append nChars chars to buf, growing it if necessary */
void
append_realloc_buf(ReallocBuf *buf, const char chars[], size_t nChars)
{
  size_t minSize = buf->index + nChars;
  if (minSize > buf->size) {
    size_t size2 = buf->size * 2;
    size_t newSize = (minSize > size2) ? minSize : size2;
    buf->buf = reallocChk(buf->buf, newSize);
    buf->size = newSize;
  }
  memcpy(buf->buf + buf->index, chars, nChars);
  buf->index += nChars;
}

/** Return base-address of current buffer. */
char *
get_realloc_buf(const ReallocBuf *buf)
{
  return buf->buf;
}

/** Return # of occupied chars in buf */
size_t
size_realloc_buf(const ReallocBuf *buf)
{
  return buf->index;
}

/** Clear buffer buf. */
void
clear_realloc_buf(ReallocBuf *buf)
{
  buf->index = 0;
}

#ifdef TEST_REALLOC_BUF

#include "unit-test.h"

int
main(int argc, const char *argv[])
{
  enum { INIT_SIZE = 4 };
  ReallocBuf *bufs[] = {
    new_sized_realloc_buf(INIT_SIZE), new_realloc_buf(),
  };
  for (int i = 0; i < sizeof(bufs)/sizeof(bufs[0]); i++) {
    //i == 0: small INIT_SIZE to force realloc's.
    ReallocBuf *buf = bufs[i];
    UNIT_TEST(size_realloc_buf(buf) == 0);
    append_realloc_buf(buf, "ab", 2);
    if (i == 0) UNIT_TEST(buf->size == INIT_SIZE);
    UNIT_TEST(size_realloc_buf(buf) == 2);
    char *buf0 = get_realloc_buf(buf);
    append_realloc_buf(buf, "cd", 2);
    //no realloc yet
    if (i == 0) UNIT_TEST(buf->size == INIT_SIZE);
    UNIT_TEST(size_realloc_buf(buf) == 4);
    if (i == 0) UNIT_TEST(get_realloc_buf(buf) == buf0); //no realloc() yet
    UNIT_TEST(strncmp("abcd", get_realloc_buf(buf), 4) == 0);
    append_realloc_buf(buf, "ef", 2);
    UNIT_TEST(strncmp("abcdef", get_realloc_buf(buf), 6) == 0);
    if (i == 0) UNIT_TEST(buf->size == INIT_SIZE*2);
    clear_realloc_buf(buf);
    UNIT_TEST(size_realloc_buf(buf) == 0);
    free_realloc_buf(buf);
  }
  report_unit_test(stderr);
  return (num_fail_unit_test() != 0);
}

#endif //ifdef TEST_REALLOC_BUF
