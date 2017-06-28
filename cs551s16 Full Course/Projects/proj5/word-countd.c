#include "common.h"
#include "worker.h"

#include "memalloc.h"

#include <assert.h>
#include <errno.h>
#include <limits.h>
#include <signal.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include <netdb.h>
#include <netinet/in.h>
#include <pthread.h>
#include <sys/socket.h>
#include <unistd.h>

//#define DO_TRACE 1
#include "trace.h"

struct ServerState {
  int port;
  int serverSocket;
  FILE *requestsLog;
  FILE *errorsLog;
};

struct ReqArg {
  struct ServerState *serverStateP;
  int clientSocket;
};

struct SignalArg {
  struct ServerState *serverStateP;
  pthread_t daemonThread;
};

/****************************** Logging ********************************/

#define LOG_DELIM "|"
#define REQUESTS_LOG "requests.log"
#define ERRORS_LOG "errors.log"

static void
do_logl(FILE *log, int intArg, ...)
{
  time_t now = time(NULL);
  struct tm *tP = gmtime(&now);
  size_t iso8601Len = strlen("YYYY-MM-DDThh:mm:ssZ");
  char iso8601[iso8601Len + 1];
  strftime(iso8601, sizeof(iso8601), "%Y-%m-%dT%H:%M:%SZ", tP);
  va_list args;
  va_start(args, intArg);
  pthread_mutex_t log_mutex = PTHREAD_MUTEX_INITIALIZER;
  int err = pthread_mutex_lock(&log_mutex);
  if (err != 0) {
    fprintf(stderr, "cannot lock log mutex: %s", strerror(err));
  }
  fprintf(log, "%s", iso8601);
  const char *p;
  while ((p = va_arg(args, const char *)) != NULL) {
    fprintf(log, "%s%s", LOG_DELIM, p);
  }
  va_end(args);
  if (intArg >= 0) {
    fprintf(log, "%s%d", LOG_DELIM, intArg);
  }
  fprintf(log, "\n");
  fflush(log);
  err = pthread_mutex_unlock(&log_mutex);
  if (err != 0) {
    fprintf(stderr, "cannot unlock log mutex: %s", strerror(err));
  }
}

static void
do_logv(FILE *log, int intArg, const char *args[])
{
  time_t now = time(NULL);
  struct tm *tP = gmtime(&now);
  size_t iso8601Len = strlen("YYYY-MM-DDThh:mm:ssZ");
  char iso8601[iso8601Len + 1];
  strftime(iso8601, sizeof(iso8601), "%Y-%m-%dT%H:%M:%SZ", tP);
  pthread_mutex_t log_mutex = PTHREAD_MUTEX_INITIALIZER;
  int err = pthread_mutex_lock(&log_mutex);
  if (err != 0) {
    fprintf(stderr, "cannot lock log mutex: %s", strerror(err));
  }
  fprintf(log, "%s", iso8601);
  for (const char **p = &args[0]; *p != NULL; p++) {
    fprintf(log, "%s%s", LOG_DELIM, *p);
  }
  if (intArg >= 0) {
    fprintf(log, "%s%d", LOG_DELIM, intArg);
  }
  fprintf(log, "\n");
  fflush(log);
  err = pthread_mutex_unlock(&log_mutex);
  if (err != 0) {
    fprintf(stderr, "cannot unlock log mutex: %s", strerror(err));
  }
}

static inline long
get_millis(struct timespec *timeP)
{
  return timeP->tv_sec * 1000 + timeP->tv_nsec/(1000*1000);
}

static void
log_request(struct ServerState *stateP, struct timespec *start,
            struct timespec *end, const char *request)
{
  char *p;
  int nArgs = (int)strtol(request, &p, 10);
  const char *requestArgs[nArgs + 1];
  TRACE("log_request(): nArgs = %d", nArgs);
  for (int i = 0; i < nArgs; i++) {
    p++; //point past NUL char
    requestArgs[i] = p;
    TRACE("log_request: request_args[%d] = %s", i, requestArgs[i]);
    p += strlen(p); //point to terminating NUL char
  }
  requestArgs[nArgs] = NULL;
  int elapsed_millis = (int)(get_millis(end) - get_millis(start));
  do_logv(stateP->requestsLog, elapsed_millis, requestArgs);
}

/***************************** Server State ****************************/

#define STREAM_PROTOCOL  "tcp"
static int
make_server_socket(int port)
{
  struct protoent *protoP = getprotobyname(STREAM_PROTOCOL);
  if (protoP == NULL) return -1;
  int s = socket(PF_INET, SOCK_STREAM, protoP->p_proto);
  if (s < 0) return -2;
  struct sockaddr_in sin;
  memset((char *)&sin, 0, sizeof(sin));
  sin.sin_family= AF_INET;
  sin.sin_addr.s_addr = INADDR_ANY;
  sin.sin_port= htons((unsigned short)port);
  if (bind(s, (struct sockaddr *)&sin, sizeof(sin)) < 0) return -3;
  enum { QLEN = 5 };
  if (listen(s, QLEN) < 0) return -4;
  return s;
}

static struct ServerState *
new_server_state(int port)
{
  FILE *errors = fopen(ERRORS_LOG, "a");
  if (!errors) {
    fprintf(stderr, "cannot open %s: %s\n", ERRORS_LOG, strerror(errno));
    exit(1);
  }
  int socket = make_server_socket(port);
  if (socket < 0) {
    fprintf(stderr, "cannot create server socket on port %d: %s\n",
            port, strerror(errno));
    do_logl(errors, -1, "cannot create server socket", strerror(errno), NULL);
    exit(1);
  }
  do_logl(errors, port, "created server socket on port", NULL);
  FILE *requests = fopen(REQUESTS_LOG, "w");
  if (!requests) {
    fprintf(stderr, "cannot append to %s: %s\n", REQUESTS_LOG, strerror(errno));
    do_logl(errors, -1, "cannot open requests log", REQUESTS_LOG,
        strerror(errno), NULL);
  }
  struct ServerState *stateP = mallocChk(sizeof(struct ServerState));
  stateP->port = port;
  stateP->serverSocket = socket;
  stateP->errorsLog = errors;
  stateP->requestsLog = requests;
  return stateP;
}

static void
free_server_state(void *p)
{
  struct ServerState *stateP = (struct ServerState *)p;
  if (close(stateP->serverSocket) < 0) {
    do_logl(stateP->errorsLog, -1, "cannot close server socket",
            strerror(errno), NULL);
  }
  if (fclose(stateP->requestsLog) != 0) {
    do_logl(stateP->errorsLog, -1, "cannot close requests log", strerror(errno),
            NULL);
  }
  if (fclose(stateP->errorsLog) != 0) {
    fprintf(stderr, "cannot close errors log: %s\n", strerror(errno));
  }
  free(stateP);
}

/*************************** Signal Handling ***************************/

static void
sigterm(int s)
{
  assert(s == SIGTERM);
}

static void *
signal_thread(void *arg)
{
  struct SignalArg *signalArgP = (struct SignalArg *)arg;
  struct ServerState *stateP = signalArgP->serverStateP;
  sigset_t catch_sigs;
  sigemptyset(&catch_sigs);
  sigaddset(&catch_sigs, SIGTERM);
  int sig_num;
  int err = sigwait(&catch_sigs, &sig_num);
  if (err != 0) {
    do_logl(stateP->errorsLog, -1, "sigwait() error", strerror(errno), NULL);
    exit(1);
  }
  do_logl(stateP->errorsLog, -1, "cancellation thread saw signal", NULL);
  assert(sig_num == SIGTERM);
  err = pthread_cancel(signalArgP->daemonThread);
  if (err != 0) {
    do_logl(stateP->errorsLog, -1, "pthread_cancel() error", strerror(errno),
            NULL);
    exit(1);
  }
  free(arg);
  do_logl(stateP->errorsLog, (int)getpid(), "exiting daemon pid", NULL);
  pthread_exit(NULL);
  return NULL;
}

/** called by daemon thread to set up signal architecture; since this is
 *  called during daemon initialization, it is set up to exit on errors
 */
static void
signal_setup(struct ServerState *stateP)
{
  TRACE("signal_setup() entry");
  //Block all signals in daemon and worker threads
  sigset_t blocked;
  sigfillset(&blocked);
  int err = pthread_sigmask(SIG_SETMASK, &blocked, NULL);
  if (err != 0) {
    do_logl(stateP->errorsLog, -1, "cannot block all signals", strerror(errno),
            NULL);
    fprintf(stderr, "cannot block all signals: %s\n", strerror(errno));
    exit(1);
  }
  TRACE("blocked all signals");

  //Set up handler for SIGTERM
  sigset_t empty;
  sigemptyset(&empty);
  struct sigaction act;
  memset(&act, 0, sizeof(act));
  act.sa_handler = sigterm;
  act.sa_mask = empty;
  act.sa_flags = 0;
  err = sigaction(SIGTERM, &act, NULL);
  if (err != 0) {
    do_logl(stateP->errorsLog, -1, "cannot set up SIGTERM handler",
            strerror(errno), NULL);
    fprintf(stderr, "cannot set up SIGTERM handler: %s\n", strerror(errno));
    exit(1);
  }
  TRACE("set up signal handler for SIGTERM");

  //Create new thread for handling signals
  struct SignalArg *signalArgP = mallocChk(sizeof(struct SignalArg));
  signalArgP->daemonThread = pthread_self();
  signalArgP->serverStateP = stateP;
  pthread_t signalThread;
  err = pthread_create(&signalThread, NULL, signal_thread, signalArgP);
  if (err != 0) {
    do_logl(stateP->errorsLog, -1, "cannot create signal thread",
            strerror(errno), NULL);
    fprintf(stderr, "cannot create signal thread: %s\n", strerror(errno));
    exit(1);
  }
  err = pthread_detach(signalThread);
  if (err != 0) {
    do_logl(stateP->errorsLog, -1, "cannot detach signal thread",
            strerror(errno), NULL);
    fprintf(stderr, "cannot detach signal thread: %s\n", strerror(errno));
    exit(1);
  }
  TRACE("created detached signal thread");
}

/*************************** Request Processing ************************/

/** Read till EOF.  Return dynamically allocated buffer, NULL on
 *  error; if n is not NULL, set *n to # of chars read before EOF or
 *  error.
 */
static char *
read_to_eof(int fd, int *n)
{
  enum { BUF_INC = 8 };
  int nRead = 0;
  size_t bufSize = 0;
  char *buf = NULL;
  bool isError = false;
  do {
    bufSize += BUF_INC;
    buf = reallocChk(buf, bufSize);
    int n = read(fd, &buf[nRead], BUF_INC);
    if (n <= 0) {
      isError = n < 0;
      break;
    }
    nRead += n;
    TRACE("nRead = %d; bufSize = %zu; read(): %d", nRead, bufSize, n);
  } while (1);
  if (isError) {
    free(buf);
    buf = NULL;
  }
  if (!isError && n != NULL) *n = nRead;
  TRACE("read %d bytes", nRead);
  return buf;
}


static void *
do_request(void *arg)
{
  struct ReqArg *reqArgP = (struct ReqArg *)arg;
  int socket = reqArgP->clientSocket;
  struct ServerState *stateP = reqArgP->serverStateP;
  free(arg);
  struct timespec start;
  TRACE("starting clock");
  int err = clock_gettime(CLOCK_REALTIME, &start);
  TRACE("start clock ret: %d", err);
  if (err != 0) {
    do_logl(stateP->errorsLog, -1, "cannot get start time",
            strerror(err), NULL);
    return NULL;
  }
  char *request = read_to_eof(socket, NULL);
  TRACE("request = %s", request);
  if (!request) {
    TRACE("about to log null request");
    do_logl(stateP->errorsLog, -1, "cannot read request", strerror(errno), NULL);
    close(socket);
    return NULL;
  }
  FILE *out = fdopen(socket, "w");
  if (!out) {
    do_logl(stateP->errorsLog, -1, "cannot open FILE stream for client",
            strerror(errno), NULL);
    return NULL;
  }
  TRACE("calling do_work(%p, %p)", request, out);
  do_work(request, out);
  struct timespec end;
  err = clock_gettime(CLOCK_REALTIME, &end);
  if (err != 0) {
    do_logl(stateP->errorsLog, -1, "cannot get end time", strerror(err), NULL);
    return NULL;
  }
  log_request(stateP, &start, &end, request);
  free(request);
  fclose(out);
  pthread_exit(NULL);
  return NULL;
}

/** Create a detached worker thread */
static void
setup_worker(struct ServerState *stateP, int clientSocket)
{
  struct ReqArg *argP = mallocChk(sizeof(struct ReqArg));
  argP->clientSocket = clientSocket;
  argP->serverStateP = stateP;
  pthread_t worker;
  int err = pthread_create(&worker, NULL, do_request, argP);
  if (err != 0) {
    do_logl(stateP->errorsLog, -1, "cannot create worker thread", strerror(err),
            NULL);
    return;
  }
  TRACE("created worker thread");
  err = pthread_detach(worker);
  if (err != 0) {
    do_logl(stateP->errorsLog, -1, "cannot detach worker thread", strerror(err),
            NULL);
    return;
  }
  TRACE("detached worker thread");
}

/***************************** Daemon Code *****************************/

/** Daemon service loop */
static void
daemon_serve(struct ServerState *stateP)
{
  signal_setup(stateP);
  //Setup clean up for daemon thread
  pthread_cleanup_push(free_server_state, stateP);
  while (1) { //infinite service loop
    struct sockaddr_in rsin;
    socklen_t rlen = sizeof(rsin);
    int s = accept(stateP->serverSocket, (struct sockaddr*)&rsin, &rlen);
    if (s < 0) {
      fprintf(stderr, "bad request on FIFO\n");
    }
    TRACE("got client connection on socket %d from %08x",
          s, rsin.sin_addr.s_addr);
    setup_worker(stateP, s); //only daemon thread returns
  }
  pthread_cleanup_pop(1);
}


static pid_t
make_daemon(struct ServerState *stateP) {
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
    do_logl(stateP->errorsLog, (int)getpid(), "created server with pid", NULL);
    daemon_serve(stateP); //should not return
    do_logl(stateP->errorsLog, -1, "unexpected daemon exit\n", NULL);
    exit(1);
  }
  return pid;
}

/*************************** Main Program ******************************/

int
main(int argc, const char *argv[])
{
  if (argc != 3) {
    fprintf(stderr, "usage: %s DIR_NAME PORT\n", argv[0]);
    exit(1);
  }
  if (chdir(argv[1]) != 0) {
    fprintf(stderr, "cannot chdir to %s: %s\n", argv[1], strerror(errno));
    exit(1);
  }
  int port = get_port(argv[2]);
  struct ServerState *stateP = new_server_state(port);

  #ifdef NO_DAEMON_OR_THREAD
  daemon_serve(stateP);
  #else
  pid_t pid = make_daemon(stateP);
  printf("%ld\n", (long)pid);
  #endif

  free_server_state(stateP);
  return 0;
}
