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
typedef int (IsWordChar)(int c);

#define IS_WORD_CHAR "isWordChar"

#define FIFO_MODE (S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH)
#define CS_FIFO_NAME "/tmp/cs"
#define SC_FIFO_NAME "/tmp/sc"
#define DELIMITOR "*"
#define MAXLINE 4096


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
daemonWorking(int serverFD);
void
makeDaemon(char * argv, int serverFD);
void
myServerOpen(int *serverFD, int *dummyFD);
void
createFIFO();
void
changeDirect(char *dir);
 
#endif
