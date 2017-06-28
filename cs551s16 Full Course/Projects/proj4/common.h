#ifndef _COMMON_H
#define _COMMON_H

#include "data-buffer.h"

#include <stdbool.h>
#include <stddef.h>
#include <semaphore.h>
#include <sys/types.h>

enum { SHM_SIZE = 1 << 12 };

/** Request semaphores: allocated within global well-known request shm */
enum {
  REQUEST_MEM_SEM,     /** 1 means request shm is available */
  HAS_REQUEST_SEM,     /** 1 means request shm contains client request */
  SEEN_REQUEST_SEM,    /** 1 means server has read client request */
  N_REQUEST_SEMS       /** Number of global sems. */
};

struct Request {
  sem_t sems[N_REQUEST_SEMS]; /** semaphores used for synchronizing request */
  struct DataBuffer request;  /** request contents */
};

/** Response semaphores: allocated within client-specific response shm */
enum {
  HAS_RESPONSE_SEM,    /** 1 means shm contains response packet for client */
  SEEN_RESPONSE_SEM,   /** 1 means client has read response packet */
  N_RESPONSE_SEMS      /** # of semaphores used */
};

struct Response {
  pid_t pid;                    /** pid of creating process */
  sem_t sems[N_RESPONSE_SEMS];  /** semaphores for synchronizing response */
  struct DataBuffer response;   /** response contents */
};

/** Return shared memory for request. The oflags argument should
 *  specify one of O_RDONLY, O_RDWR or O_WRONLY, or'd with permissible
 *  optional flags like O_CREAT, etc.  If oflags|O_CREAT, then create
 *  shared memory if necessary with permissions specified by mode.
 *
 *  Returns address at which memory attached.  If error, then terminate
 *  program with an appropriate error message.
 */
struct Request *open_request_shm(int oflags, mode_t mode);

/** Free all resources occupied by shm in calling process.  If
 *  doUnlink then unlink shared memory
 */
void close_request_shm(struct Request *shm, bool doUnlink);

/** Return shared memory for response for client with PID pid. The
 *  oflags argument should specify one of O_RDONLY, O_RDWR or
 *  O_WRONLY, or'd with permissible optional flags like O_CREAT, etc.
 *  If oflags|O_CREAT, then create shared memory if necessary with
 *  permissions specified by mode.
 *
 *  Returns address at which memory attached.  If error, then terminate
 *  program with an appropriate error message.
 */
struct Response *open_response_shm(pid_t pid, int oflags, mode_t mode);

/** Free all resources occupied by shm.  If doUnlink then unlink shared
 *  memory
 */
void close_response_shm(struct Response *shm, bool doUnlink);


#endif //ifndef _COMMON_H
