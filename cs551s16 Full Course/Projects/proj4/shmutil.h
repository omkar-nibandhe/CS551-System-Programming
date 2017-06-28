#ifndef _SHMUTIL_H
#define _SHMUTIL_H

#include <stddef.h>
#include <sys/stat.h>


/** Return a POSIX shm object for posixName with specified size and
 *  specified oflags (one of O_RDONLY, O_RDWR or O_WRONLY, or'd with
 *  permissible optional flags like O_CREAT, etc).
 *
 *  Returns address at which memory attached.  If error, then terminate
 *  program with an appropriate error message.
 */
void *getPosixShm(const char *posixName, int oflags, mode_t mode, size_t size);

/** Unmap POSIX shm with name posixName at addr with specified size;
 *  if doUnlink is non-zero, also unlink POSIX shm with specified
 *  posixName.
 */
void disposePosixShm(const char *posixName, void *addr, 
		     size_t size, int doUnlink);

#endif
