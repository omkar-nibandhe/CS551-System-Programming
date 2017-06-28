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

enum { MAX_BUF = 100 };
enum { QLEN = 5 };

int main(int argc, char *argv[])
{
	
  int port;
  int s;
  struct sockaddr_in sin;
  if (argc != 2 || 
      (port = atoi(argv[1])) < 1024) {
    fprintf(stderr, "%s PORT\n", argv[0]); exit(1);
  }
  if ((s = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
    perror("socket"); exit(1);
  }
memset(&sin, 0, sizeof(sin));
  sin.sin_family = AF_INET;
  sin.sin_addr.s_addr = INADDR_ANY;
  sin.sin_port = htons((unsigned short)port);

  if (bind(s, (struct sockaddr *)&sin, 
	   sizeof(sin)) < 0) {
    perror("bind"); exit(1);
  }
  if (listen(s, QLEN) < 0) {
    perror("listen"); exit(1);
  }
	
  while (1) {
    struct sockaddr_in rsin;
    socklen_t rlen = sizeof(rsin);
    char buf[MAX_BUF];
    FILE *rf;
    int rs = accept(s, (struct sockaddr*)&rsin, &rlen);
    if (rs < 0) {
      perror("accept"); exit(1);	
    }
    if (!(rf = fdopen(rs, "r"))) {
      perror("fdopen"); exit(1);
    }
    while ((fgets(buf, MAX_BUF, rf))) {
      write(rs, "->oniband1 ", 4); 
      write(rs, buf, strlen(buf));
    }
  }

}

