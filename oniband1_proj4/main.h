#ifndef MAIN_HEADER
#define MAIN_HEADE

#include <stdio.h>
#include <stdlib.h>
#include <error.h>
#include <sys/types.h>
#include <sys/stat.h> 
#include <fcntl.h>
#include <string.h>
#include <signal.h>
#include <errno.h>
#include <unistd.h>
#include <dlfcn.h>
#include <semaphore.h>
#include <string.h>

#define DELIMITOR "*"
#define MAXLINE 4096
enum {
  SERVER_SEM,
  REQUEST_SEM,
 // RESPONSE_SEM,
  N_SEMS
};

void
validateArguments(int argc);
char **
myTokenizer(char *buff);
int
openClientFIFO(char *pid);
void 
workerProcess(char *buff);
void
doubleFork(char *buff);
void
daemonWorking();
void
makeDaemon(char * argv);
void
myServerOpen(int *serverFD, int *dummyFD);
void
changeDirect(char *dir);
 
#endif
