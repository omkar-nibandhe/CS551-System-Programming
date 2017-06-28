#include <semaphore.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#define LOGINLENGTH 32

void
concatinator( char * loginName, char * semaphoreName, char * finalResult){
	int length1=0, length2=0;
	length1 = strlen(loginName);
	length2 = strlen(semaphoreName);
	finalResult = realloc(finalResult,(length1 + length2 + 2) * sizeof(char));
		if(finalResult == NULL){
			fprintf(stderr,"melloc fail\n.");
			exit(0);
		}
	strcpy(finalResult, "//");
	strcpy(finalResult, loginName);
	strcat(finalResult, semaphoreName);
	fprintf(stderr,"\n finalResult concatinator :%s\n",finalResult);
}
#ifdef TEST
int main(){

	char * loginName = malloc(sizeof(char)*LOGINLENGTH);
	//fprintf(stderr,"\n%s",getlogin());
	loginName = getlogin();
	char * semaphoreName = "/myName";
	char * finalResult = malloc(sizeof(char)) ;
	fprintf(stderr,"\n 111finalResult concatinator :%s\n",finalResult);
	concatinator(loginName, semaphoreName, finalResult);

	return 0;
}
#endif
