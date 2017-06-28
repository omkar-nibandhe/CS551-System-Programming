#define MAX_BUF 4096
#define POSIX_IPC_NAME_PREFIX "/oniband1"
#define SHM_NAME POSIX_IPC_NAME_PREFIX "shm"
#define SERVER_SEM_NAME POSIX_IPC_NAME_PREFIX "server"
#define REQUEST_SEM_NAME POSIX_IPC_NAME_PREFIX "request"
#define RESPONSE_SEM_NAME POSIX_IPC_NAME_PREFIX "response"
#define ALL_RW_PERMS    (S_IRUSR|S_IWUSR|S_IRGRP|S_IWGRP|S_IROTH|S_IWGRP)

#include "main.h"
#include "previous_main.c"
#include <sys/mman.h>
#include "lock.c"
#include "semaphore.h"

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
 /* { .posixName = RESPONSE_SEM_NAME, 
    .oflags = O_RDWR|O_CREAT, 
    .mode = ALL_RW_PERMS,
    .initValue = 0,
  },
 */
};
//#include "previous_main.c"
//
// Main Program
//

void
validateArguments(int argc){
	if(argc != 2){
		fprintf(stderr,"\nInvalid number of arguments:\nexpected\t./word-countd DIR_NAME\n");
		exit(0);
	}
}

char **
myTokenizer(char *buff){
	//fprintf(stderr,"\n--%s\n",buff);
  	int bufsize = MAXLINE, i = 0;
  	char **tokens = malloc(bufsize * sizeof(char*));
  	char *token;
  	if (!tokens) {
  	  //fprintf(stderr, "Cannot allocate memory\n");
	exit(1);
  	  //exit1(0,"Cannot allocate memory\n");
  	}
  	token = strtok(buff, DELIMITOR);
  	while (token != NULL) {
  	  tokens[i] = token;
  	  i++;
   	 if (i >= bufsize) {
     	 bufsize += MAXLINE;
     	 tokens = realloc(tokens, bufsize * sizeof(char*));
      	if (!tokens) {
       	 //fprintf(stderr, "lsh: allocation error\n");
	exit(1);//exit1(0," allocation error\n");
       	 exit(EXIT_FAILURE);
      	}	
    }

    token = strtok(NULL,DELIMITOR);
  }
  tokens[i] = NULL;
  return tokens;
}



void 
workerProcess(char *buff){

	char **argument = myTokenizer(buff);
	int numberWords = atoi(argument[0]);
	int i=0,pid_count;
	while(argument[i] != NULL){
		//fprintf(stderr,"\n%d,argument:%s",i,argument[i]);
		i++;
	}
	//i=0;
	pid_count = --i;	//PID
	strcpy(clientpid,argument[pid_count]);
	if(argument[0] != 0)
	previous_main(i,argument);
	else
	fprintf(stderr,"0 Words.\n");
	//strcpy(bufResp,"response_SEM_Name");
	//sem_post(responseHandler);

	//close(clientRDWR);
	//unlink(argument[pid_count]);
	exit(0);
	
}

void
doubleFork(char *buff){
	//fprintf(stderr,"double fork[%s]",buff);
	
 	pid_t pid=0;
  		if ( (pid = fork()) < 0){
    			fprintf(stderr,"fork error");
			exit(0);
		}
  	else if (pid == 0) {         
    		if ( (pid = fork()) < 0){
     			fprintf(stderr,"fork error");
			exit(0);
		}
    		else if (pid > 0)
      			exit(0); 
		workerProcess(buff);
    		
  	}
}

void
daemonWorking(){
	//int value; 
     	//sem_getvalue(&SERVER, &value); 
      	//printf("The value of the semaphors is %d\n", value);
	sem_t *sems[N_SEMS];
	int i=0;
	for ( i = 0; i < N_SEMS; i++) {
    	const SemOpenArgs *p = &semArgs[i];
    		if ((sems[i] = sem_open(p->posixName, p->oflags, p->mode, p->initValue))== NULL) {
      			fprintf(stderr,"cannot create semaphore %s:", p->posixName);
    		}
    	//SEM_VALUE("server", "after init", sems[i], p->posixName);
 	 }

	int fd = shm_open(SHM_NAME, O_RDWR|O_CREAT, ALL_RW_PERMS);
  	if (fd < 0) fprintf(stderr,"cannot create shm %s:", SHM_NAME);
  	if (ftruncate(fd, MAX_BUF) < 0) {
   		 fprintf(stderr,"cannot size shm %s to %d:", SHM_NAME, MAX_BUF);
  	}

  	char *buf = NULL;
  	if ((buf = mmap(NULL, MAX_BUF, PROT_READ|PROT_WRITE, MAP_SHARED, fd, 0))== MAP_FAILED) {
    		fprintf(stderr,"cannot mmap shm %s:", SHM_NAME);
  	}
  	//fprintf(stderr, "memory attached at %p\n", buf);
	while(1){ 

		if (sem_wait(sems[REQUEST_SEM]) < 0) {
		      fprintf(stderr,"wait error on sem %s:", REQUEST_SEM_NAME);
    			}
		//fprintf(stderr,"Server:%s\n",buf);
		doubleFork(buf);
	

	}
}
void
makeDaemon(char * argv){
	 pid_t pid=0,sid=0;
   	 pid = fork ( );  
   		 if (pid < 0){  
       			fprintf(stderr,"\nFork failed.\n");
			exit(0);
		}
    		 if (pid > 0)  {
			fprintf(stderr,"\nDaemon Pid:[%d]\t",pid);
        		exit(0);  
	
		}
		
	umask(0);
	sid=setsid();	
		if(sid < 0){
        		fprintf(stderr,"\nSession failed.\n");
			exit(0);
		}
	if(lockPidFile() != 0){
		fprintf(stderr,"\nLocking Session failed.\n");
		exit(1);

	}
    	if ((chdir (".")) < 0) {
		fprintf(stderr,"\nDir error.\n");
			exit(0);
		} 
}

void
changeDirect(char *dir){
	if ((chdir(dir)) < 0) {
		fprintf(stderr,"\nDir error.\n");
			exit(0);
		} 
}

/*void
intisem(sem_t *name){
	if(sem_init(name,1,1) < 0){
		fprintf(stderr,"\nsem_init error : %s",(char *)name);
		exit(1);
	}

}
*/
int 
main(int argc, char *argv[]) {
	
	validateArguments(argc);
	changeDirect(argv[1]);
	/*sem_t *sems[N_SEMS];
	for (int i = 0; i < N_SEMS; i++) {
	    const SemOpenArgs *p = &semArgs[i];
	    if ((sems[i] = sem_open(p->posixName, p->oflags, p->mode, p->initValue))== NULL) {
		      fprintf(stderr,"cannot create semaphore %s:", p->posixName);
    		}
	    SEM_VALUE("server", "after init", sems[i], p->posixName);
  	}
	*/
	makeDaemon(argv[1]);
	daemonWorking();

	
    	return 0; 

}




