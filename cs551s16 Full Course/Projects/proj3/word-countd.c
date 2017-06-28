#include "client-server-defs.h"
#include "worker.h"

#include <errno.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

static FILE *
open_response_fifo(const char *clientPid)
{
  char clientPidName[strlen(CLIENT_FIFO_PREFIX) + strlen(clientPid) + 1];
  sprintf(clientPidName, "%s%s", CLIENT_FIFO_PREFIX, clientPid);
  FILE *out = fopen(clientPidName, "w");
  return out;
}

static void
do_request(const char request[])
{
  const char *clientPid = request;
  FILE *out = open_response_fifo(clientPid);
  if (!out) {
    fprintf(stderr, "cannot open response fifo for client pid %s: %s\n",
            clientPid, strerror(errno));
  }
  do_work(request + strlen(clientPid) + 1, out);
  fclose(out);
}

/** Create a worker process using the double-fork technique.  Have
 *  worker process request[] using do_request().
 */
static void
setup_worker(FILE *requests, const char request[])
{
  pid_t child_pid = fork();
  if (child_pid < 0) {
    fprintf(stderr, "daemon child fork error: %s\n", strerror(errno));
  }
  else if (child_pid == 0) { //daemon child
    fclose(requests);
    pid_t worker_pid = fork();
    if (worker_pid < 0) {
      fprintf(stderr, "daemon child fork error: %s\n", strerror(errno));
    }
    else if (worker_pid == 0) { //worker process
      do_request(request);
      exit(0);
    }
    else { //daemon child
      exit(0);
    }
  }
  else { //daemon process
    int status;
    if (waitpid(child_pid, &status, 0) < 0) {
      fprintf(stderr, "daemon wait error: %s\n", strerror(errno));
    }
  }
}

/** Read the next NUL or EOF terminated string from in into buf
 *  having size bufSize.  Normally will store the string chars + the
 *  terminating NUL (if any) in buf.  However, it will never store
 *  more than bufSize chars in buf[].  It returns # of chars it would
 *  have stored (< 0) on EOF; it follows that if the return value >
 *  bufSize, then buf[] was not large enough.
 *
 *  Note that a return value < 0 or > bufSize indicates a fatal
 *  error condition on in and the stream is assumed to be dead.
 */
static int
read_string(FILE *in, char buf[], int bufSize)
{
  int n = 0;
  int c;
  char *bufEnd = buf + bufSize;
  char *p = buf;
  do {
    c = fgetc(in);
    if (c != EOF && p < bufEnd) {
      *p++ = c;
    }
    n++;
  } while (c != '\0' && c != EOF);
  return (c == EOF) ? -1 : n;
}

/** Read strings from in into buf[] with max size bufSize.
 *  The stream in should contain the following NUL-terminated strings:
 *    Client PID
 *    N_ARGS (should be at least 4)
 *    count N_ARGS remaining arguments.
 *  Return # of chars read, < 0 on error
 */
static int
read_request(FILE *in, char buf[], size_t bufSize)
{
  enum { MIN_N_ARGS = 4 };
  char *p = buf;
  size_t nLeft = bufSize;
  int pidLen = read_string(in, p, nLeft);
  if (pidLen < 0 || pidLen > nLeft) return -1;
  p += pidLen; nLeft -= pidLen;
  int nArgsLen = read_string(in, p, nLeft);
  if (nArgsLen < 0 || nArgsLen > nLeft) return -2;
  char *p1;
  long nArgs = strtol(p, &p1, 10);
  if (p1 == buf || *p1 != '\0' || nArgs < MIN_N_ARGS) return 0;
  p += nArgsLen; nLeft -= nArgsLen;
  for (int i = 0; i < nArgs; i++) {
    int n = read_string(in, p, nLeft);
    if (n < 0 || n > nLeft) return -3 - i;
    p += n; nLeft -= n;
  }
  return p - buf;
}

/** Daemon service loop */
static void
daemon_serve(FILE *requests)
{
  while (1) { //infinite service loop
    char buf[PIPE_BUF];
    int requestSize = read_request(requests, buf, sizeof(buf));
    if (requestSize < 0) {
      fprintf(stderr, "bad request on FIFO\n");
      return;
    }
    setup_worker(requests, buf); //only daemon returns
  }
}

static void
set_blocking(const char *name, int fd)
{
  int flags = fcntl(fd, F_GETFL, 0);
  if (flags < 0) {
    fprintf(stderr, "cannot get flags for  %s(%d): %s\n",
            name, fd, strerror(errno));
    exit(1);
  }
  if (fcntl(fd, F_SETFL, flags & ~O_NONBLOCK) < 0) {
    fprintf(stderr, "cannot set %s(%d) unblocking: %s\n",
            name, fd, strerror(errno));
    exit(1);
  }
}

/** Return a read-only FILE stream on fifo with name fifoName (the
 *  underlying file descriptor is opened read-write to avoid EOF when
 *  the number of clients writing the FIFO goes from 1 to 0).
 */
static FILE *
open_requests(const char *fifoName)
{
  //opening fifo O_RDWR not defined for FIFO; hence portably get
  //effect of O_RDWR by first non-blocking open O_RDONLY and then open
  //O_WRONLY.
  int fd = open(fifoName, O_RDONLY|O_NONBLOCK);
  if (fd < 0) {
    fprintf(stderr, "cannot read %s: %s\n", fifoName, strerror(errno));
    exit(1);
  }
  if (open(fifoName, O_WRONLY) < 0) { //no block since already reading
    fprintf(stderr, "cannot read %s: %s\n", fifoName, strerror(errno));
    exit(1);
  }
  set_blocking(fifoName, fd);
  FILE *requests = fdopen(fd, "r");
  if (requests == NULL) {
    fprintf(stderr, "cannot fdopen(%d) for %s: %s\n", fd, fifoName,
            strerror(errno));
    exit(1);
  }
  return requests;
}

static pid_t
make_daemon(void) {
  //open requests in invoking process so that errors can be
  //reported before forking daemon
  FILE *requests = open_requests(REQUESTS_FIFO_NAME);
  pid_t pid = fork();
  if (pid < 0) {
    fprintf(stderr, "cannot fork daemon: %s\n", strerror(errno));
    exit(1);
  }
  else if (pid == 0) {
    if (setsid() < 0) {
      fprintf(stderr, "cannot create new session: %s\n", strerror(errno));
      exit(1);
    }
    daemon_serve(requests); //should not return
    fprintf(stderr, "unexpected daemon exit\n");
    exit(1);
  }
  fclose(requests); //invoking process does not need daemon
  return pid;
}

int
main(int argc, const char *argv[])
{
  if (argc != 2) {
    fprintf(stderr, "usage: %s DIR_NAME\n", argv[0]);
    exit(1);
  }
  if (chdir(argv[1]) != 0) {
    fprintf(stderr, "cannot chdir to %s: %s\n", argv[1], strerror(errno));
    exit(1);
  }
  if (mkfifo(REQUESTS_FIFO_NAME, 0666) < 0 && errno != EEXIST) {
    fprintf(stderr, "cannot create FIFO %s: %s\n", REQUESTS_FIFO_NAME,
            strerror(errno));
    exit(1);
  }
  pid_t pid = make_daemon();
  printf("%ld\n", (long)pid);
  return 0;
}
