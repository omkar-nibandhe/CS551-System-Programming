
#include "main.h"
#include "previous_main.c"
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
  	  exit1(0,"Cannot allocate memory\n");
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
	exit1(0," allocation error\n");
       	 exit(EXIT_FAILURE);
      	}	
    }

    token = strtok(NULL,DELIMITOR);
  }
  tokens[i] = NULL;
  return tokens;
}

int
openClientFIFO(char *pid){
	int clientRDWR=0;
	clientRDWR = open (pid, O_WRONLY);
	if(clientRDWR < 0){
		//fprintf(stderr,"\nClient Pipe opening error.\n");
		exit1(0,"\nClient Pipe opening error.");
	}
	return clientRDWR;
		
}

void 
workerProcess(char *buff){
	//fprintf(stderr,"woker process[%s]",buff);
	char **argument = myTokenizer(buff);
	int numberWords = atoi(argument[1]);
	int i=0,pid_count;
	while(argument[i] != NULL){
		i++;
	}
	pid_count = --i;	//PID
	//printf("---%d:%s---",i,argument[i++]);
	strcpy(clientpid,argument[i++]);
	if(previous_main(i,argument) == 0);
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
  	else if (pid == 0) {          /* first child */
    		if ( (pid = fork()) < 0){
     			fprintf(stderr,"fork error");
			exit(0);
		}
    		else if (pid > 0)
      			exit(0);  /* parent from 2nd fork == 1st child */
	
    		/* second child. */
		//fprintf(stderr,"\nWorker Pid: [%d]\n",getpid());
		//fprintf(stderr, "\n1.%s\n",buff);
		workerProcess(buff);
    		
		//sleep(2);
    		//printf("second child, parent pid = %d\n",getppid());
    		
  	}
  	/* wait for first child */
  		if (waitpid(pid, NULL, 0) != pid)
   		 fprintf(stderr,"waitpid error");
}

void
daemonWorking(int serverFD){
	
	size_t req=0;
	char *buff=malloc(sizeof(char));
	
	while(1)
	{
		/* Read requests and send responses */
		if (-1 == read(serverFD, &req, sizeof(size_t))) {
			fprintf(stderr, "\nError reading request: discarding\n");
			exit(0);
		}
		else{
			buff[0]='\0';
			buff = realloc(buff,req*sizeof(char)+1);
				if(buff == NULL){
					fprintf(stderr, "\nMem alloc prob.\n");
					exit(0);
				}
			buff[req]='\0';
				if((-1 == read(serverFD, buff, req*sizeof(char))) && errno != EAGAIN){
					fprintf(stderr, "\nError reading request; discarding\n");
					exit(0);
				}
			buff[req+1]='\0';
			doubleFork(buff);
	 	}
	}
}
void
makeDaemon(char * argv, int serverFD){
	 pid_t pid=0,sid=0;
   	 pid = fork ( );  
   		 if (pid < 0){  
       			fprintf(stderr,"\nFork failed.\n");
			exit(0);
		}
    		 if (pid > 0)  
        		exit (0);  
		
	umask(0);
	fprintf(stderr,"\nDaemon Pid:[%d]\n",getpid());
	sid=setsid();	
		if(sid < 0){
        		fprintf(stderr,"\nSession failed.\n");
			exit(0);
		}
    			
	
    
    	if ((chdir (".")) < 0) {
		fprintf(stderr,"\nDir error.\n");
			exit(0);
		} 
}

void
myServerOpen(int *serverFD, int *dummyFD){
	*serverFD = open("SERVER_FIFO", O_RDWR);
	if (*serverFD == -1){
		fprintf(stderr,"\nError opening FIFO\n");		
		exit(0);
		}
	*dummyFD = open("SERVER_FIFO", O_WRONLY);
	if (*dummyFD == -1){
		fprintf(stderr,"\nError opening FIFO\n");		
		exit(0);
		}
}
void
createFIFO(){
	umask(0);
	
	if (mkfifo("SERVER_FIFO", FIFO_MODE) == -1 && errno != EEXIST)
		exit(0);
}



void
changeDirect(char *dir){
	if ((chdir(dir)) < 0) {
		fprintf(stderr,"\nDir error.\n");
			exit(0);
		} 
}
int 
main(int argc, char *argv[]) {
	//validateArguments(argc);
	changeDirect(argv[1]);
    
	int serverFD=0, dummyFD=0;
	createFIFO();
	//myServerOpen(&serverFD, &dummyFD);

	serverFD = open("SERVER_FIFO", O_RDWR );
	if (serverFD == -1){
		fprintf(stderr,"\nError opening FIFO\n");		
		exit(0);
		}
	dummyFD = open("SERVER_FIFO", O_RDWR );
	if (dummyFD == -1){
		fprintf(stderr,"\nError opening FIFO\n");		
		exit(0);
		}


	makeDaemon(argv[1],serverFD);
	daemonWorking(serverFD);

	close(serverFD);
	
    	return 0; 

}




