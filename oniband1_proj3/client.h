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
#define FIFO_MODE (S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH)

#define CS_FIFO_NAME "/tmp/cs"
#define SC_FIFO_NAME "/tmp/sc"
#define DELIMITOR "*"



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
mywrite(int argc, char **argv, int writefd);
void
myread(int openClientFIFO, char *numberofwords);
void
changeDirect(char *dir);
#endif

/*
1. background check on file1 file2 file3 file4



*/
