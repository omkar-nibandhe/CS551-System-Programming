#include "shmdemo.h"

#include <errors.h>

#include <fcntl.h>
#include <gdbm-ndbm.h>
#include <semaphore.h>
#include <stdio.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>

#define ERROR "error"
#define NOT_FOUND "not found"
#define OK "ok"

#define DBM_NAME "mydbm"

typedef struct {
  const char *posixName;
  int oflags;
  mode_t mode;
  unsigned initValue;
} SemOpenArgs;

static SemOpenArgs semArgs[] = {
  { .posixName = SERVER_SEM_NAME, 
    .oflags = O_RDWR|O_CREAT, 
    .mode = ALL_RW_PERMS,
    .initValue = 1,
  },
  { .posixName = REQUEST_SEM_NAME, 
    .oflags = O_RDWR|O_CREAT, 
    .mode = ALL_RW_PERMS,
    .initValue = 0,
  },
  { .posixName = RESPONSE_SEM_NAME, 
    .oflags = O_RDWR|O_CREAT, 
    .mode = ALL_RW_PERMS,
    .initValue = 0,
  },
};

int 
main(int argc, const char *argv[])
{
  DBM *dbm = dbm_open(DBM_NAME, O_RDWR | O_CREAT, 0660);
  sem_t *sems[N_SEMS];
  if (!dbm) fatal("could not open %s:", DBM_NAME);
  
  for (int i = 0; i < N_SEMS; i++) {
    const SemOpenArgs *p = &semArgs[i];
    if ((sems[i] = sem_open(p->posixName, p->oflags, p->mode, p->initValue)) 
	== NULL) {
      fatal("cannot create semaphore %s:", p->posixName);
    }
    SEM_VALUE("server", "after init", sems[i], p->posixName);
  }


  int fd = shm_open(SHM_NAME, O_RDWR|O_CREAT, ALL_RW_PERMS);
  if (fd < 0) fatal("cannot create shm %s:", SHM_NAME);
  if (ftruncate(fd, MAX_BUF) < 0) {
    fatal("cannot size shm %s to %d:", SHM_NAME, MAX_BUF);
  }

  char *buf = NULL;
  if ((buf = mmap(NULL, MAX_BUF, PROT_READ|PROT_WRITE, MAP_SHARED, fd, 0))
      == MAP_FAILED) {
    fatal("cannot mmap shm %s:" SHM_NAME);
  }
  fprintf(stderr, "memory attached at %p\n", buf);

  while (1) { //server loop
    datum resp = { OK, strlen(OK) };
    SEM_VALUE("server", "before wait", sems[REQUEST_SEM], REQUEST_SEM_NAME);
    if (sem_wait(sems[REQUEST_SEM]) < 0) {
      fatal("wait error on sem %s:", REQUEST_SEM_NAME);
    }
    switch (buf[0]) {
      case '?': {
	datum key = { buf + 1, strlen(buf) - 1 };
	resp = dbm_fetch(dbm, key);
	if (resp.dptr == NULL) {
	  resp.dptr = NOT_FOUND; resp.dsize = strlen(NOT_FOUND);
	}
      }
      break;
      case '+': {
	char *const p = strchr(buf, ' ');
	datum key = { buf + 1, p - (buf + 1) };
	datum content = { p + 1, strlen(buf) - (p - buf) };
	if (dbm_store(dbm, key, content, DBM_REPLACE) < 0) {
	  resp.dptr = ERROR; resp.dsize = strlen(ERROR);
	}
      }
      break;
      case '-': {
	datum key = { buf + 1, strlen(buf) - 1 };
	if (dbm_delete(dbm, key) < 0) {
	  resp.dptr = ERROR; resp.dsize = strlen(ERROR);
	}
      }
      break;
      default:
        resp.dptr = ERROR; resp.dsize = strlen(ERROR);
    } /* switch (buf[0]) */
    sprintf(buf, "%.*s", resp.dsize, resp.dptr);
    if (sem_post(sems[RESPONSE_SEM]) < 0) {
      fatal("cannot post sem %s:", RESPONSE_SEM_NAME);
    }
    SEM_VALUE("server", "after post", sems[RESPONSE_SEM], RESPONSE_SEM_NAME);
  } // while (1) server loop
  return 0;
}
