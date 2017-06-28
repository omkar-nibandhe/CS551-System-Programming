#include "data-buffer.h"

#include "memalloc.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>

/** Return a new dynamically allocated data buffer */
struct DataBuffer *
new_data_buffer(size_t size)
{
  struct DataBuffer *dataP = mallocChk(sizeof(struct DataBuffer) + size);
  dataP->isError = false;
  dataP->count = 0;
  dataP->size = size;
  return dataP;
}

/** Free a previously allocated data buffer */
void
free_data_buffer(struct DataBuffer *data)
{
  free(data);
}

/** Append up to count chars from buf into DataBuffer pointed to by dataP.
 *  Return number of chars copied.
 */
size_t
append_chars_to_data_buffer(const char buf[], size_t count,
                            struct DataBuffer *dataP)
{
  size_t nLeft = dataP->size - dataP->count;
  size_t nCopy = (nLeft < count) ? nLeft : count;
  memcpy(&dataP->data[dataP->count], buf, nCopy);
  dataP->count += nCopy;
  return nCopy;
}

/** Append long v as text to data.  Return # of chars appended, < 0 if
 *  it did not fit.
 */
int
append_long_data_buffer(long v, struct DataBuffer *dataP)
{
  int nLeft = dataP->size - dataP->count;
  int n = snprintf(&dataP->data[dataP->count], nLeft, "%ld", v);
  if (n > nLeft) return -1;
  dataP->count += n;
  return n;
}
