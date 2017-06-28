#include "errors.h"

#include <stdlib.h>

void *mallocChk(size_t size)
{
  void *p = malloc(size);
  if (!p) fatal("malloc failure:");
  return p;
}

void *reallocChk(void *ptr, size_t size) 
{
  void *p = realloc(ptr, size);
  if (!p) fatal("realloc failure:");
  return p;
}

void *callocChk(size_t nmemb, size_t size) 
{
  void *p = calloc(nmemb, size);
  if (!p) fatal("calloc failure:");
  return p;
}
