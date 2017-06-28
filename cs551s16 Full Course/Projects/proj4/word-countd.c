#include "common.h"
#include "worker.h"

#include "errors.h"

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

//#define DO_TRACE 1
#include "trace.h"

#define PROG_TRACE(...)  TRACE("server: " __VA_ARGS__)

static void
send_response_data(struct Response *response)
{
  PROG_TRACE("sending response (%d bytes) %.*s", (int)response->response.count,
             (int)response->response.count, response->response.data);
  if (sem_post(&response->sems[HAS_RESPONSE_SEM]) < 0) {
    error("cannot signal for HAS_RESPONSE_SEM:");
  }
  if (sem_wait(&response->sems[SEEN_RESPONSE_SEM]) < 0) {
    error("cannot wait on SEEN_RESPONSE_SEM:");
  }
}

static void
do_request(const char request[])
{
  char *p;
  pid_t clientPid = (pid_t)strtol(request, &p, 10);
  const char *restRequest = p + 1;
  struct Response *response = open_response_shm(clientPid, O_RDWR, 0666);
  struct Worker *worker = new_worker(restRequest, &response->response);
  PROG_TRACE("created worker object %p", worker);
  if (worker == NULL) { //error creating worker: response has error message
    send_response_data(response);
  }
  else {
    while (next_response(worker, &response->response) != 0) {
      send_response_data(response);
    }
  }
  //send empty packet to signal end of response
  response->response.count = 0;
  send_response_data(response);
  if (worker != NULL) free_worker(worker);
  close_response_shm(response, false);
}

/** Create a worker process using the double-fork technique.  Have
 *  worker process request[] using do_request().
 */
static void
setup_worker(const char request[])
{
  PROG_TRACE("setup_worker(): daemon pid = %ld", (long)getpid());
  pid_t child_pid = fork();
  if (child_pid < 0) {
    fprintf(stderr, "daemon child fork error: %s\n", strerror(errno));
  }
  else if (child_pid == 0) { //daemon child
    pid_t worker_pid = fork();
    if (worker_pid < 0) {
      fprintf(stderr, "daemon child fork error: %s\n", strerror(errno));
    }
    else if (worker_pid == 0) { //worker process
      PROG_TRACE("created worker pid = %ld", (long)getpid());
      do_request(request);
      PROG_TRACE("exit worker pid = %ld", (long)getpid());
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


/** Daemon service loop */
static void
daemon_serve(struct Request *request)
{
    char buf[request->request.size];
    while (1) { //infinite service loop
      PROG_TRACE("pid: %ld: waiting for request", (long)getpid());
      if (sem_wait(&request->sems[HAS_REQUEST_SEM]) < 0) {
        error("cannot wait for HAS_REQUEST_SEM:");
      }
      PROG_TRACE("HAS_REQUEST_SEM wait returned");
      memcpy(buf, request->request.data, request->request.count);
      PROG_TRACE("got request from client (%d bytes) %s",
                 (int)request->request.count, buf);
      if (sem_post(&request->sems[SEEN_REQUEST_SEM]) < 0) {
        error("cannot signal for SEEN_REQUEST_SEM:");
      }
      setup_worker(buf); //only daemon call returns
    }
}



static pid_t
make_daemon(void) {
  //open requests in invoking process so that errors can be
  //reported before forking daemon
  struct Request *request = open_request_shm(O_RDWR|O_CREAT, 0666);
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
    daemon_serve(request); //should not return
    fprintf(stderr, "unexpected daemon exit\n");
    exit(1);
  }
  close_request_shm(request, false); //invoking process does not need shm
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
  pid_t pid = make_daemon();
  printf("%ld\n", (long)pid);
  return 0;
}
