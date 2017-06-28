#include "errors.h"
#include "shmutil.h"

#include <stddef.h>

#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

/** Return a POSIX shm object for posixName with specified size and
 *  specified oflags (one of O_RDONLY, O_RDWR or O_WRONLY, or'd with
 *  permissible optional flags like O_CREAT, etc).
 *
 *  Returns address at which memory attached.  If error, then terminate
 *  program with an appropriate error message.
 */
void *
getPosixShm(const char *posixName, int oflags, mode_t mode, size_t size)
{
  int fd = shm_open(posixName, oflags, mode);
  if (fd < 0) {
    fatal("cannot create shm %s:", posixName);
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
void 
disposePosixShm(const char *posixName, void *addr, size_t size, int doUnlink)
{
  if (munmap(addr, size) < 0) {
    fatal("cannot unmap shm %s at %p:", posixName, addr);
  }
  if (doUnlink && shm_unlink(posixName) < 0) {
    fatal("cannot unlink shm %s:", posixName);
  }
}


