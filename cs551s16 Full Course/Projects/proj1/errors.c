#include <errno.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "errors.h"

static int nErrors= 0;

/* Print a error message on stderr as per printf-style fmt and
 * optional arguments.  If fmt ends with ':', follow with a space,
 * strerror(errno), followed by a newline.
 */
void
error(const char *fmt, ...)
{
  va_list ap;
  va_start(ap, fmt);
  vfprintf(stderr, fmt, ap);
  if (fmt[strlen(fmt) - 1] == ':') fprintf(stderr, " %s\n", strerror(errno));
  nErrors++;
}

/* Print a error message on stderr as per printf-style fmt and
 * optional arguments.  If fmt ends with ':', follow with a space,
 * strerror(errno), followed by a newline.  Terminate the program.
 */
void
fatal(const char *fmt, ...)
{
  va_list ap;
  va_start(ap, fmt);
  vfprintf(stderr, fmt, ap);
  if (fmt[strlen(fmt) - 1] == ':') fprintf(stderr, " %s\n", strerror(errno));
  fprintf(stderr, "terminating execution...\n");
  exit(1);
}

/* Return # of errors seen so far. */
int 
getErrorCount(void)
{
  return nErrors;
}
