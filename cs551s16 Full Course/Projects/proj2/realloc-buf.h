#ifndef _REALLOC_BUF_H
#define _REALLOC_BUF_H

#include <stddef.h>

/** A realloc'd buffer which is double'd in size whenever it is too small.
 */

typedef struct ReallocBufImpl ReallocBuf;

/** Return a new realloc-buf with initial size initSize. */
ReallocBuf *new_sized_realloc_buf(size_t initSize);

/** Return a new realloc-buf with default initial size. */
ReallocBuf *new_realloc_buf(void);

/** Free all resources used by buf */
void free_realloc_buf(ReallocBuf *buf);

/** Append nChars chars to buf, growing it if necessary */
void append_realloc_buf(ReallocBuf *buf, const char chars[], size_t nChars);

/** Return base-address of current buffer. */
char *get_realloc_buf(const ReallocBuf *buf);

/** Return # of occupied chars in buf */
size_t size_realloc_buf(const ReallocBuf *buf);

/** Clear buffer buf. */
void clear_realloc_buf(ReallocBuf *buf);

#endif //ifndef _REALLOC_BUF_H
