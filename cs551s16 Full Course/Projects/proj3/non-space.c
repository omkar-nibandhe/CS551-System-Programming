#include "word-char.h"

#include "ctype.h"

int
isWordChar(int c)
{
  return !isspace(c);
}
