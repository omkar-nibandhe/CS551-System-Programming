#ifndef clien
#define clien


#include <stdio.h>
#include <stdlib.h>
#include <error.h>
#include <sys/types.h>
#include <sys/stat.h> 
#include <fcntl.h>
#include <string.h>
#include <errno.h>
#include <sys/mman.h>
#include <semaphore.h>
#include <unistd.h>

#define FIFO_MODE (S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH)
//#define SHM_NAME POSIX_IPC_NAME_PREFIX "/oniband1"
#define POSIX_IPC_NAME_PREFIX "/oniband1"
#define SHM_NAME POSIX_IPC_NAME_PREFIX "shm"
#define SERVER_SEM_NAME POSIX_IPC_NAME_PREFIX "server"
#define REQUEST_SEM_NAME POSIX_IPC_NAME_PREFIX "request"
#define RESPONSE_SEM_NAME POSIX_IPC_NAME_PREFIX "response"
#define ALL_RW_PERMS    (S_IRUSR|S_IWUSR|S_IRGRP|S_IWGRP|S_IROTH|S_IWGRP)
#define MAX_BUF 4096
#define CS_FIFO_NAME "/tmp/cs"
#define SC_FIFO_NAME "/tmp/sc"
#define DELIMITOR "*"
enum {
  SERVER_SEM,
  REQUEST_SEM,
 // RESPONSE_SEM,
  N_SEMS
};



void
validateArguments(int argc, char * argv[]);
int 
myopen();
void
myclose(int a); 
void
createClientFIFO();
int
openClientFIFO();
void
mywrite(int argc, char **argv, char * writefd);
void
myread(int openClientFIFO, char *numberofwords);
void
changeDirect(char *dir);

static void 
semWait(sem_t *sem, const char *posixName);
static void 
semPost(sem_t *sem, const char *posixName);

#endif

/*
1. background check on file1 file2 file3 file4



*/
