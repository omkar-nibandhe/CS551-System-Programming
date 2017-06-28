#include "common.h"
#include "errors.h"
#include "memalloc.h"

#include <errno.h>
#include <math.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <fcntl.h>
#include <semaphore.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/types.h>
#include <unistd.h>

enum {
  MAX_REQUEST = SHM_SIZE - offsetof(struct Request, request.data),
  MAX_RESPONSE = SHM_SIZE - offsetof(struct Response, response.data),
};

#define WELL_KNOWN_SUFFIX "_REQUESTS"

/** Return dynamically allocated POSIX name of well-known shm name */
static const char *
make_request_shm_name(void)
{
  const char *login = getlogin();
  size_t nameLen = 1 + strlen(login) + strlen(WELL_KNOWN_SUFFIX);
  char *name = mallocChk(nameLen + 1);
  sprintf(name, "/%s%s", login, WELL_KNOWN_SUFFIX);
  return name;
}

#define RESPONSE_SUFFIX "_RESPONSE"

/** Return dynamically allocated POSIX name of local shm name
 *  corresponding to pid
 */
static const char *
make_response_shm_name(pid_t pid)
{
  const char *login = getlogin();
  size_t nameLen =
    1 + strlen(login) + 1 + floor(log10(pid)) + 1 + strlen(RESPONSE_SUFFIX);
  char *name = mallocChk(nameLen + 1);
  sprintf(name, "/%s-%ld%s", login, (long)pid, RESPONSE_SUFFIX);
  return name;
}

/** Return a POSIX shm object for posixName with specified size and
 *  specified oflags (one of O_RDONLY, O_RDWR or O_WRONLY, or'd with
 *  permissible optional flags like O_CREAT, etc).
 *
 *  Returns address at which memory attached.  If error, then terminate
 *  program with an appropriate error message.
 */
static void *
mmap_posix_shm(const char *posixName, int oflags, mode_t mode, size_t size)
{
  int fd = shm_open(posixName, oflags, mode);
  if (fd < 0) {
    fatal("cannot access shm %s:", posixName);
  }
  if ((oflags & O_CREAT) && ftruncate(fd, size) < 0) {
    fatal("cannot truncate shm %s to %d:", posixName, size);
  }
  int prot =
      (oflags & O_RDONLY) ? PROT_READ
    : (oflags & O_WRONLY) ? PROT_WRITE
    : (PROT_READ|PROT_WRITE);
  void *addr = mmap(NULL, size, prot, MAP_SHARED, fd, 0);
  if (addr == MAP_FAILED) {
    fatal("cannot map shm %s of size %d:", posixName, size);
  }
  return addr;
}

/** Unmap POSIX shm with name posixName at addr with specified size;
 *  if doUnlink is non-zero, also unlink POSIX shm with specified
 *  posixName.
 */
static void
close_posix_shm(const char *posixName, void *addr, size_t size, bool doUnlink)
{
  if (munmap(addr, size) < 0) {
    fatal("cannot unmap shm %s at %p:", posixName, addr);
  }
  if (doUnlink && shm_unlink(posixName) < 0) {
    fatal("cannot unlink shm %s:", posixName);
  }
}

/** Return shared memory for request. The oflags argument should
 *  specify one of O_RDONLY, O_RDWR or O_WRONLY, or'd with permissible
 *  optional flags like O_CREAT, etc.  If oflags|O_CREAT, then create
 *  shared memory if necessary with permissions specified by mode.
 *
 *  Returns address at which memory attached.  If error, then terminate
 *  program with an appropriate error message.
 */
struct Request *
open_request_shm(int oflags, mode_t mode)
{
  const char *name = make_request_shm_name();
  struct Request *shm = mmap_posix_shm(name, oflags, mode, SHM_SIZE);
  if (oflags & O_CREAT) {
    shm->request.size = MAX_REQUEST;
    for (int i = 0; i < N_REQUEST_SEMS; i++) {
      int v = (i == 0) ? 1 : 0;
      if (sem_init(&shm->sems[i], 1, v) < 0) {
        fatal("cannot initialize request semaphore %d:", i);
      }
    }
  }
  free((void *)name);
  return shm;
}

/** Free all resources occupied by shm.  If doUnlink then unlink shared
 *  memory
 */
void
close_request_shm(struct Request *shm, bool doUnlink)
{
  const char *name = make_request_shm_name();
  close_posix_shm(name, shm, SHM_SIZE, doUnlink);
  free((void *)name);
}

/** Return shared memory for response for client with PID pid. The
 *  oflags argument should specify one of O_RDONLY, O_RDWR or
 *  O_WRONLY, or'd with permissible optional flags like O_CREAT, etc.
 *  If oflags|O_CREAT, then create shared memory if necessary with
 *  permissions specified by mode.
 *
 *  Returns address at which memory attached.  If error, then terminate
 *  program with an appropriate error message.
 */
struct Response *
open_response_shm(pid_t pid, int oflags, mode_t mode)
{
  const char *name = make_response_shm_name(pid);
  struct Response *shm = mmap_posix_shm(name, oflags, mode, SHM_SIZE);
  if (oflags & O_CREAT) {
    shm->pid = pid;
    shm->response.size = MAX_RESPONSE;
    for (int i = 0; i < N_RESPONSE_SEMS; i++) {
      if (sem_init(&shm->sems[i], 1, 0) < 0) {
        fatal("cannot initialize global semaphore %d:", i);
      }
    }
  }
  free((void *)name);
  return shm;
}

/** Free all resources occupied by shm.  If doUnlink then unlink shared
 *  memory
 */
void
close_response_shm(struct Response *shm, bool doUnlink)
{
  const char *name = make_response_shm_name(shm->pid);
  close_posix_shm(name, shm, SHM_SIZE, doUnlink);
  free((void *)name);
}
