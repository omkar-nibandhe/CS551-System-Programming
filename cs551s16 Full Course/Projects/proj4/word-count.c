#include "common.h"
#include "errors.h"
#include "memalloc.h"

#include <errno.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

//#define DO_TRACE 1
#include "trace.h"

#define PROG_TRACE(...)  TRACE("client: " __VA_ARGS__)

static void
append_null_to_request(struct DataBuffer *dataP)
{
  if (append_chars_to_data_buffer("\0", 1, dataP) != 1) {
    fprintf(stderr, "cannot append NUL to request: count %zu, size %zu\n",
            dataP->count, dataP->size);
  }
}


/** Assemble request from args[nArgs] into buf.  Precede args[] arguments
 *  by pid and nArgs.
 */
static void
assemble_request(int nArgs, const char *args[], struct DataBuffer *dataP)
{
  dataP->count = 0;
  if (append_long_data_buffer((long)getpid(), dataP) < 0) {
    fprintf(stderr, "cannot append pid %ld to request: count %zu; size %zu\n",
            (long)getpid(), dataP->count, dataP->size);
    exit(1);
  }
  append_null_to_request(dataP);
  if (append_long_data_buffer(nArgs, dataP) < 0) {
    fprintf(stderr, "cannot append nArgs %d to request: count %zu; size %zu\n",
            nArgs, dataP->count, dataP->size);
    exit(1);
  }
  append_null_to_request(dataP);
  for (int i = 0; i < nArgs; i++) {
    int n = strlen(args[i]) + 1;
    if (append_chars_to_data_buffer(args[i], n, dataP) != n) {
      fprintf(stderr, "cannot append arg %d to request: count %zu; size %zu\n",
              i, dataP->count, dataP->size);
      exit(1);
    }
  }
}

static void
send_request(int nArgs, const char *args[])
{
  struct Request *request = open_request_shm(O_RDWR, 0666);
  PROG_TRACE("waiting for REQUEST_MEM_SEM");
  if (sem_wait(&request->sems[REQUEST_MEM_SEM]) < 0) {
    fatal("cannot lock REQUEST_MEM_SEM:");
  }
  assemble_request(nArgs, args, &request->request);
  PROG_TRACE("sending request (%d bytes) %s", (int)request->request.count,
             request->request.data);
  if (sem_post(&request->sems[HAS_REQUEST_SEM]) < 0) {
    fatal("cannot post HAS_REQUEST_SEM:");
  }
  PROG_TRACE("posted HAS_REQUEST_SEM, waiting on SEEN_REQUEST_SEM");
  if (sem_wait(&request->sems[SEEN_REQUEST_SEM]) < 0) {
    fatal("cannot lock SEEN_REQUEST_SEM:");
  }
  if (sem_post(&request->sems[REQUEST_MEM_SEM]) < 0) {
    fatal("cannot post REQUEST_MEM_SEM:");
  }
  close_request_shm(request, false);
  PROG_TRACE("initial request sent");
}

/** Return # of lines output, < 0 if error occurs */
static int
out_response(struct Response *response, FILE *out)
{
  bool isError = false;
  int nLines = 0;
  int n = 0;
  do { //while non-empty packets
    PROG_TRACE("waiting for HAS_RESPONSE_SEM");
    if (sem_wait(&response->sems[HAS_RESPONSE_SEM]) < 0) {
      fatal("cannot wait on HAS_RESPONSE_SEM:");
    }
    n = response->response.count;
    isError = isError || response->response.isError;
    FILE *o = (response->response.isError) ? stderr : out;
    PROG_TRACE("isError = %d; got response (%d bytes) %.*s", isError,
               n, n, response->response.data);
    fprintf(o, "%.*s", n, response->response.data);
    for (const char *p = strchr(response->response.data, '\n');
         p != NULL && p < &response->response.data[n];
         p++) {
      nLines++;
    }
    PROG_TRACE("posting for SEEN_RESPONSE_SEM");
    if (sem_post(&response->sems[SEEN_RESPONSE_SEM]) < 0) {
      fatal("cannot wait on SEEN_RESPONSE_SEM:");
    }
  } while (n > 0);
  return (isError) ? -1 : nLines;
}

int
main(int argc, const char *argv[])
{
  if (argc < 5) {
    fprintf(stderr,
            "usage: %s DIR_NAME N STOP_WORDS FILE1...\n",
            argv[0]);
    exit(1);
  }
  if (chdir(argv[1]) != 0) {
    fprintf(stderr, "cannot chdir to %s: %s\n", argv[1], strerror(errno));
    exit(1);
  }
  struct Response *response = open_response_shm(getpid(), O_RDWR|O_CREAT, 0666);
  send_request(argc - 2, &argv[2]);
  int nWords = out_response(response, stdout);
  close_response_shm(response, true);
  return (nWords < 0);
}
