#include <ctype.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <arpa/inet.h>
#include <netdb.h>
#include <netinet/in.h>
#include <strings.h>
#include <sys/socket.h>
#include <unistd.h>

#include "common.c"
#include "concatination.c"
#define LOCALHOST "127.0.0.1"
enum { MAX_BUF = 100 };

int 
main(int argc, char *argv[])
{
	validateArgumentClient(argc,argv);
	//changeDirect(argv[1]);
		
 	int port;
  	int s;
  	struct sockaddr_in sin;

  	if ((port = atoi(argv[2])) < 1024) {
   		 fprintf(stderr, "%s HOST-ADDR PORT\n", argv[0]); exit(1);
 	 }
  	if ((s = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
    		perror("socket"); exit(1);
  	}
  	memset(&sin, 0, sizeof(sin));
  	sin.sin_family = AF_INET;
	if(strcmp(argv[1], LOCALHOST) != 0){
		strcpy(argv[1], LOCALHOST);
	}
  	if (inet_pton(AF_INET, argv[1], &sin.sin_addr.s_addr) <= 0) {
   		fprintf(stderr, "cannot convert %s:", argv[1]);
    		exit(1);
  	}
  
	sin.sin_port = htons((unsigned short)port);

  	if (connect(s, (const struct sockaddr*)&sin, sizeof(sin)) < 0) {
    		fprintf(stderr, "cannot connect:");
    		exit(1);
  	}

	mywrite(argc,argv,s);
	myread(atoi(argv[3]),s);

	close(s);
	/*
  	char *line = NULL;
  	int lineSize = 0;
  	int nRead;
  	FILE *in = fdopen(s, "r");
  	if (in == NULL) {
    		fprintf(stderr, "cannot open file on socket %d:", s);
    		exit(1);
  	}
  	while ((nRead = getline(&line, &lineSize, stdin)) > 0) {
    int nWrite = 0;
    while (nWrite < nRead) {
      int n = write(s, line + nWrite, nRead - nWrite);
      if (n <= 0) break;
      nWrite += n;
    }
    if (nWrite < nRead) {
      fprintf(stderr, "could not write");
      exit(1);
    }
    if (getline(&line, &lineSize, in) > 0) {
      printf("%s", line);
    }
  }
  free(line);
	*/
}
