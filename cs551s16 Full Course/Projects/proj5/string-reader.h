#ifndef _STRING_READER_H
#define _STRING_READER_H

#include "realloc-buf.h"
#include <stdio.h>

/** Read next string from FILE stream f open on file with name
 *  fileName, having only chars for which isStringChar() returns
 *  non-zero.  Use realloc'd buffer buf for storing the string.
 *  Return pointer to NUL-terminated string.  Return NULL on EOF.
 */
char *read_string(FILE *f, const char *fileName,
                  ReallocBuf *buf, int (*isStringChar)(int c));

#endif //ifndef _STRING_READER_H
