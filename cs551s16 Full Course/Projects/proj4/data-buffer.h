#ifndef _DATA_BUFFER_H
#define _DATA_BUFFER_H

#include <stdbool.h>
#include <stddef.h>

struct DataBuffer {
  size_t size;       /** size of data[] */
  size_t count;      /** # of chars currently in data[] */
  bool isError;      /** true if data[] contents reports an error */
  char data[];       /** actual data */
};

/** Return a new dynamically allocated data buffer */
struct DataBuffer *new_data_buffer(size_t size);

/** Free a previously allocated data buffer */
void free_data_buffer(struct DataBuffer *data);

/** Append up to count chars from buf into DataBuffer pointed to by dataP.
 *  Return number of chars copied.
 */
size_t append_chars_to_data_buffer(const char buf[], size_t count,
                                   struct DataBuffer *dataP);

/** Append long v as text to data.  Return # of chars appended, < 0 if
 *  it did not fit.
 */
int append_long_data_buffer(long v, struct DataBuffer *dataP);

#endif //#ifndef _DATA_BUFFER_H
