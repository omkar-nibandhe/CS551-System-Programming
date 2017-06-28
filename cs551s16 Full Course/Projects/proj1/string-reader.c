#include "errors.h"
#include "realloc-buf.h"
#include "string-reader.h"

#include <stdio.h>

/** Read next string from f having only chars for which isStringChar()
 *  returns non-zero.  Use realloc'd buffer buf for storing the string.
 *  Return pointer to NUL-terminated string.  Return NULL on EOF.
 */
char *
read_string(FILE *f, const char *fileName,
            ReallocBuf *buf, int (*isStringChar)(int c))
{
  static int c = '\0'; //remember last char read
  clear_realloc_buf(buf);
  if (c != EOF) {
    do {
      c = fgetc(f);
    } while (c != EOF && !isStringChar(c));
  }
  if (c != EOF) {
    char chars[1];
    while (c != EOF && isStringChar(c)) {
      chars[0] = c;
      append_realloc_buf(buf, chars, sizeof(chars));
      c = fgetc(f);
    }
    chars[0] = '\0';
    append_realloc_buf(buf, chars, sizeof(chars));
    return get_realloc_buf(buf);
  }
  else if (feof(f)) {
    c = '\0';
    return NULL;
  }
  else {
    fatal("i/o error reading file %s:", fileName);
  }
}

#ifdef TEST_STRING_READER

//Test code uses non-standard string streams.

#include "unit-test.h"

#include <ctype.h>
#include <stdio.h>
#include <string.h>

#define WORD1 "abc"
#define WORD2 "#@$%"
#define WORD3 "ass3;#"
#define LINEAR_SPACE " \t"
#define SPACE LINEAR_SPACE "\n"

static int
isWordChar(int c)
{
  return !isspace(c);
}

static void
test_file(const char *string, const char *fileName,
          const char *words[], int nWords)
{
  //fmemopen() fails if 2nd arg is 0, hence workaround to open /dev/null.
  FILE *f =
    (strlen(string) == 0)
    ? fopen("/dev/null", "r")
    : fmemopen((char *)string, strlen(string), "r");
  if (!f) {
    fatal("cannot open file %s with contents \"%s\":", fileName, string);
  }
  int wordsIndex = 0;
  ReallocBuf *wordBuf = new_sized_realloc_buf(16);
  for (const char *word = read_string(f, fileName, wordBuf, isWordChar);
       word != NULL;
       word = read_string(f, fileName, wordBuf, isWordChar)) {
    UNIT_TEST(strcmp(word, words[wordsIndex]) == 0);
    clear_realloc_buf(wordBuf);
    wordsIndex++;
  }
  free_realloc_buf(wordBuf);
  UNIT_TEST(wordsIndex == nWords);
  if (fclose(f) != 0){
    fatal("cannot close %s:", fileName);
  }
}

static void
test_content_file(void)
{
  const char *words[] = { WORD1, WORD2, WORD3, WORD2, WORD3, WORD1 };
  const int nWords = sizeof(words)/sizeof(words[0]);
  enum { WORDS_PER_NL = 3 };
  ReallocBuf *buf = new_realloc_buf();
  for (int i = 0; i < sizeof(words)/sizeof(words[0]); i++) {
    const char *space = ((i % WORDS_PER_NL) == 0) ? SPACE : LINEAR_SPACE;
    append_realloc_buf(buf, space, strlen(space));
    append_realloc_buf(buf, words[i], strlen(words[i]));
  }
  append_realloc_buf(buf, "\0", 1);
  const char *string = get_realloc_buf(buf);
  test_file(string, "content-test-file", words, nWords);
  free_realloc_buf(buf);
}


static void
test_empty_file(void)
{
  test_file("", "empty-file", NULL, 0);
}

static void
test_whitespace_file(void)
{
  const char *string = SPACE LINEAR_SPACE;
  test_file(string, "whitespace-file", NULL, 0);
}

int
main(int argc, const char *argv[])
{
  test_content_file();
  test_empty_file();
  test_whitespace_file();
  report_unit_test(stderr);
  return (num_fail_unit_test() != 0);
}

#endif //ifdef TEST_STRING_READER
