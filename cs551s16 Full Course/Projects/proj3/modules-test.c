#include "word-char.h"

#include <dlfcn.h>
#include <stdio.h>
#include <stdlib.h>

typedef int (IsWordChar)(int c);

#define IS_WORD_CHAR "isWordChar"

static void
test_word_char(const char *text, IsWordChar *isWordChar, FILE *out)
{
  int inWord = 0;
  for (const char *p = text; *p != '\0'; p++) {
    int c = *p;
    if (isWordChar(c)) {
      fputc(c, out);
      inWord = 1;
    }
    else if (inWord) {
      fputc('\n', out);
      inWord = 0;
    }
  }
  if (inWord) fputc('\n', out);
}

int
main(int argc, const char *argv[])
{
  if (argc < 3) {
    fprintf(stderr, "usage: %s MODULE_NAME TEXT_ARG...", argv[0]);
    exit(1);
  }
  const char *modName = argv[1];
  void *dlHandle = dlopen(modName, RTLD_NOW);
  if (dlHandle == NULL) {
    fprintf(stderr, "cannot open dynamic module %s: %s\n", modName, dlerror());
    exit(1);
  }
  IsWordChar *isWordChar = dlsym(dlHandle, IS_WORD_CHAR);
  if (isWordChar == NULL) {
    fprintf(stderr, "cannot resolve %s in %s: %s\n", IS_WORD_CHAR, modName,
            dlerror());
    exit(1);
  }
  for (int i = 2; i < argc; i++) {
    test_word_char(argv[i], isWordChar, stdout);
  }
  if (dlclose(dlHandle) != 0) {
    fprintf(stderr, "cannot close module %s: %s\n", modName, dlerror());
    exit(1);
  }
  return 0;
}
