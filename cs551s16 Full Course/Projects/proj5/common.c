#include "common.h"

#include <stdio.h>
#include <stdlib.h>

/** Convert portString to a non-root port.  Exit with error if not valid.  */
int
get_port(const char *portString)
{
  char *p;
  int port = (int)strtol(portString, &p, 10);
  enum { MIN_PORT = 1024, MAX_PORT = (1<<16) - 1 };
  if (port < MIN_PORT || port > MAX_PORT || *p != '\0') {
    fprintf(stderr, "bad port %s: must be int in interval [%d, %d]\n",
            portString, MIN_PORT, MAX_PORT);
    exit(1);
  }
  return port;
}
