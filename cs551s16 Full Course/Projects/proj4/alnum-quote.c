#include "word-char.h"

#include "ctype.h"

int
isWordChar(int c)
{
  return isalnum(c) || c == '\'';
}
