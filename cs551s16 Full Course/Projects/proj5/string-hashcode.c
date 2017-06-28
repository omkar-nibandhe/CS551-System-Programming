#include <limits.h>
#include <stdlib.h>

static int didInit = 0;

enum { N_CHARS = (1 <<CHAR_BIT) };
static int rands[N_CHARS];

static void
do_init(void)
{
  for (int i = 0; i < N_CHARS; i++) {
    rands[i] = rand();
  }
}

int
string_hashcode(void *string)
{
  const char *s = (const char *)string;
  if (!didInit) {
    do_init();
    didInit = 1;
  }
  int hash = 0;
  for (const char *p = s; *p != '\0'; p++) {
    hash += rands[*(unsigned char *)p];
  }
  return hash;
}

#ifdef TEST_STRING_HASHCODE

#include <stdio.h>

int
main(int argc, const char *argv[])
{
  for (int i = 0; i < argc; i++) {
    printf("hashcode(\"%s\") = %d\n", argv[i], string_hashcode(argv[i]));
  }
  return 0;
}

#endif //ifdef TEST_STRING_HASHCODE
