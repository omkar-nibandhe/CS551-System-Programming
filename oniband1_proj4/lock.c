
//#define CHECK

#include<stdio.h>
#include<stdlib.h>
#include<fcntl.h>
#include<errno.h>
#define FILE_NAME ".pid1"	
static int
lockPidFile()
{
  int fd = open(FILE_NAME, O_WRONLY|O_CREAT, 0644);
  if (fd < 0) {
    fprintf(stderr, "cannot open %s\n", FILE_NAME);
    exit(1);
  }
  else {
    struct flock flock;
    flock.l_type = F_WRLCK;
    flock.l_whence = SEEK_SET;
    flock.l_start = 0;
    flock.l_len = 0;
    if (fcntl(fd, F_SETLK, &flock) < 0) {
      if (errno == EAGAIN &&
          fcntl(fd, F_GETLK, &flock) >= 0) {
        fprintf(stderr, "process %ld has lock\n",(long)flock.l_pid);
      }
      else {
        fprintf(stderr, "lock error: \n");
      }
      exit(1);
    }
    if (ftruncate(fd, 0) != 0) {
      fprintf(stderr, "cannot truncate %s\n",FILE_NAME);
      exit(1);
    }
    {
      FILE *f = fdopen(fd, "w");
      if (!f) {
        fprintf(stderr, "cannot fdopen %s(%d)\n", FILE_NAME, fd);
        exit(1);
      }
      fprintf(f, "%ld\n", (long)getpid());
      fflush(f);
    }
  }
  return 0;
}
#ifdef CHECK
int 
main() {
  printf("%ld trying lock\n", (long)getpid());
  lockPidFile();
  printf("%ld got lock\n", (long)getpid());
  sleep(60);
  return 0;
}
#endif 
