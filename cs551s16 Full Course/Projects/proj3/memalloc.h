#ifndef _MEMALLOC_H
#define _MEMALLOC_H

/** Simple wrappers around malloc() and friends which terminate entire
 *  program with a fatal error on failure.
 */

#include <stdlib.h>

void *mallocChk(size_t size);

void *reallocChk(void *ptr, size_t size);

void *callocChk(size_t nmemb, size_t size);

#endif //ifndef _MEMALLOC_H
