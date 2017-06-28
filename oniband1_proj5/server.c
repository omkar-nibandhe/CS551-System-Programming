
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
#include <arpa/inet.h>
#include <netdb.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <pthread.h>

#define ERRLOG "errors.log"
#define REQLOG "request.log"
#define MAXLINE 4096
#define MAX_BUF 4096
#define QLEN 10
#include "daemon.c"
#include "common.c"
#include "thread.c"
#include "logs.c"
#include "previous_main.c"
#include "signal.c"
typedef struct workerarguments{
	int writefd;
	char * list;
}workerarguments;

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

void *
workerProcess( void * arg){
	//fprintf(stderr,"~~~~~~~hello~~~~~~%d size\n%s\n",strlen((char*)arg),(char*)arg);
	//struct workerarguments *a = (struct workerarguments *) arg;
	//fprintf(stderr,"woker process[%d]",a->writefd, a->list);

	//pthread_exit();
	char **argument = myTokenizer(arg);
	int numberWords = atoi(argument[1]);
	int i=0,pid_count;
	while(argument[i] != NULL){
		//fprintf(stderr,"\n arg[%d] = %s",i,argument[i]);
		i++;
	}
	pid_count = --i;	//PID
	logentry(argument,i-1);
	time_t start, end;
	start = clock();

	if(++i >= 5)
	if(previous_main(i,argument) == 0);
	end = clock();
	logentryend(argument,i-1,(end-start));
	pthread_exit(0);
	//close(clientRDWR);
	//unlink(argument[pid_count]);
	//exit(0);
	
}



void
daemonWorking(char * port_number){
	int port = 0 ;
  	int s = 0 ;
 	struct sockaddr_in sin;
	
	if ( (port = atoi(port_number)) < 1024) {
    		fprintf(stderr, "%s PORT\n", port_number); exit(1);
  	}
	if ((s = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
    		 exit1(0, "socket");
  	}
	memset(&sin, 0, sizeof(sin));
  	sin.sin_family = AF_INET;
  	sin.sin_addr.s_addr = INADDR_ANY;
  	sin.sin_port = htons((unsigned short)port);

  	if (bind(s, (struct sockaddr *)&sin, sizeof(sin)) < 0) {
   		exit1(0,"bind");
  	}
  	if (listen(s, QLEN) < 0) {
   		 exit1(0,"listen");
  	}
		struct sockaddr_in rsin;
    		socklen_t rlen = sizeof(rsin);
   		FILE *rf;
		int rs = 0;
		int requestlength = 0;
	while(1){
    		rs = accept(s, (struct sockaddr*)&rsin, &rlen);
    		if (rs < 0) {
      			exit1(0,"accept");
    		}
		char buf[MAX_BUF];
		buf[0]='\0';
		//fprintf(stderr,"server->%s\n",buf);
		read(rs, buf, MAX_BUF);		
			
		char digi[8];
		snprintf(digi, 8, "%d", rs);
		//fprintf(stderr, "RS: %s:\n", digi);

		strcat(buf,DELIMITOR);
		strcat(buf,digi);
		
		pthread_t pthre;
		pthread_create(&pthre, NULL, workerProcess, (void*)buf);
		pthread_detach(pthre);
    		}
}


int 
main(int argc, char *argv[]) {
	validateArguments(argc);
	changeDirect(argv[1]);
	makeDaemon(argv[1]);
	struct sigaction action;
    	memset(&action, 0, sizeof(struct sigaction));
    	action.sa_handler = term;
    	sigaction(SIGTERM, &action, NULL);
	
	createlogfile();
	serverstart();
	daemonWorking(argv[2]);
	serverstop();

}
