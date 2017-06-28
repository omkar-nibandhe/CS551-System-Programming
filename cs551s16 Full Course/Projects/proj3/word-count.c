#include "client-server-defs.h"
#include "memalloc.h"

#include <errno.h>
#include <limits.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

static inline size_t
get_pid_len(void)
{
  return log10(getpid()) + 1;
}


static char *
make_client_fifo(void)
{
  size_t pidLen = get_pid_len();
  char *clientFifoName = mallocChk(strlen(CLIENT_FIFO_PREFIX) + pidLen + 1);
  sprintf(clientFifoName, "%s%ld", CLIENT_FIFO_PREFIX, (long)getpid());
  if (mkfifo(clientFifoName, 0666) < 0) {
    fprintf(stderr, "cannot create FIFO %s: %s\n", clientFifoName,
            strerror(errno));
    exit(1);
  }
  return clientFifoName;
}

static void
remove_client_fifo(char *clientFifoName)
{
  if (unlink(clientFifoName) < 0) {
    fprintf(stderr, "cannot unlink %s: %s\n", clientFifoName, strerror(errno));
    exit(1);
  }
  free(clientFifoName);
}

/** Assemble request from args[nArgs] into buf.  Precede args[] arguments
 *  by pid and nArgs.
 */
static size_t
assemble_request(int nArgs, const char *args[], char buf[], size_t bufSize)
{
  char *p = buf;
  size_t nLeft = bufSize;
  int n = snprintf(p, bufSize, "%ld", (long)getpid());
  if (n >= nLeft) {
    fprintf(stderr, "pid %ld does not fit within %zu bytes\n",
            (long)getpid(), nLeft);
    exit(1);
  }
  p += n + 1; nLeft -= n + 1;
  n = snprintf(p, nLeft, "%d", nArgs);
  if (n >= nLeft) {
    fprintf(stderr, "# of args %d do not fit in buf[%zu]\n", nArgs, bufSize);
    exit(1);
  }
  p += n + 1; nLeft -= n + 1;
  for (int i = 0; i < nArgs; i++) {
    n = snprintf(p, nLeft, "%s", args[i]);
    if (n >= nLeft) {
      fprintf(stderr, "args[%d] does not fit in buf[%zd]\n", i, bufSize);
      exit(1);
    }
    p += n + 1; nLeft -= n + 1;
  }
  return p - buf;
}

static void
send_request(int nArgs, const char *args[])
{
  char request[PIPE_BUF];
  size_t requestSize = assemble_request(nArgs, args, request, sizeof(request));
  int out = open(REQUESTS_FIFO_NAME, O_WRONLY);
  if (out < 0) {
    fprintf(stderr, "cannot open requests FIFO %s: %s\n", REQUESTS_FIFO_NAME,
            strerror(errno));
    exit(1);
  }
  if (write(out, request, requestSize) != requestSize) {
    fprintf(stderr, "cannot write request: %s\n", strerror(errno));
    exit(1);
  }
  close(out);
}

/** Returns # of distinct words in response; < 0 on error */
static int
out_response(FILE *in, FILE *out)
{
  int nWords = 0;
  int n;
  while (fscanf(in, "%d", &n) == 1) {
    int c = fgetc(in); //skip space
    int isError = (n < 0);
    FILE *o = (isError) ? stderr : out;
    while ((c = fgetc(in)) != '\0' && c != EOF) {
      fputc(c, o);
    }
    if (c == EOF) {
      fprintf(stderr, "unexpected EOF in response\n");
      return -1;;
    }
    if (n > 0) {
      fprintf(o, " %d", n);
      nWords++;
    }
    fprintf(o, "\n");
    if (isError) return -1;
  }
  return nWords;
}

int
main(int argc, const char *argv[])
{
  if (argc < 6) {
    fprintf(stderr,
            "usage: %s DIR_NAME WORD_CHAR_MODULE N STOP_WORDS FILE1...\n",
            argv[0]);
    exit(1);
  }
  if (chdir(argv[1]) != 0) {
    fprintf(stderr, "cannot chdir to %s: %s\n", argv[1], strerror(errno));
    exit(1);
  }
  char *clientFifoName = make_client_fifo();
  send_request(argc - 2, &argv[2]);
  FILE *in = fopen(clientFifoName, "r");
  if (!in) {
    fprintf(stderr, "cannot open FIFO %s: %s\n", clientFifoName,
            strerror(errno));
    exit(1);
  }
  int nWords = out_response(in, stdout);
  fclose(in);
  remove_client_fifo(clientFifoName);
  return (nWords < 0);
}
