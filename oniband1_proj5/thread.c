#include <pthread.h>


void
threadcreate(char *function, char *buf, int fd){
	fprintf(stderr,"Thread function:%s ~~%s~~ %d",function, buf, fd);
	
	pthread_t thread;
	size_t length = strlen(buf);
	char *argument = malloc((length+10)*sizeof(char));
	strcpy(argument,buf);
	strcat(argument,DELIMITOR);	
	char d[8];
	snprintf(d,8,"%d",fd);
	strcat(argument,d);
//	fprintf(stderr,"~~~~~~~~~~~~~~~~~~~~%s\n",argument);


	if( pthread_create(&thread, NULL, (void*)function ,(void *)argument )){
		fprintf(stderr,"\nThread error. %s",function);
	}


}
