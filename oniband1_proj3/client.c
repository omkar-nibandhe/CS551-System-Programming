#include"client.h"
//#include "main.h"

void
validateArguments(int argc, char * argv[]){
	if(argc < 5){
		fprintf(stderr,"\nInvalid number of arguments:\nexpected\t./word-countd DIR_NAME\n");
		exit(0);
	}
	int i=4;
	char *fname =argv[i];
	while(i != argc){
		if( access( fname, F_OK | R_OK ) != -1 ) {
			// file exists so skip to next file
			fname=argv[++i];
	
		} else {
    			// file doesn't exist
			fprintf(stderr,"Some files missing.: %s\nExiting.\n",argv[i]);
			exit(0);
		}
	}
}

int 
myopen(){
	int writefd = open ("SERVER_FIFO", O_WRONLY );
	if(writefd == -1){
		fprintf(stderr,"\n Client ID:%d error opening FIFO.\n",getpid());
		exit(0);
	}
	return writefd;
}

void
myclose(int a){
	close(a);

}
void
createClientFIFO(){
	umask(0);
	char *d = malloc(sizeof(char)*8);
	snprintf(d,8,"%d",getpid());
	//fprintf(stderr,"\n---->%s\n",d);
	if (mkfifo(d, FIFO_MODE) == -1 && errno != EEXIST){
		exit(0);
	}
	free(d);
	d=NULL;
}
int
openClientFIFO(){
	char *d = malloc(sizeof(char)*8);
	snprintf(d,8,"%d",getpid());
	//fprintf(stderr,"\n---->%s\n",d);
	int clientRD = open(d,O_RDONLY);
	if(clientRD == 0){
		fprintf(stderr,"\nClient side client FIFO opening error. \n");
		exit(0);
	}
	//fprintf(stderr,"\nclinet fifo read: %d\n",clientRD);
	free(d);
	d=NULL;
	return clientRD;
}
void
mywrite(int argc, char **argv, int writefd){
	int i=0;
	size_t stringlength=0;
	
	for(i=2;i<argc;i++){
		stringlength += strlen(argv[i]);
		}
	//printf("\n%zd\n",stringlength);
	stringlength=(stringlength+argc+8);
	char *buff= malloc(stringlength*sizeof(char));
	char *d = malloc(sizeof(char)*8);
	snprintf(d,8,"%d",getpid());
	for(i=2;i<argc;i++){
		strcat(buff,argv[i]);
		strcat(buff,DELIMITOR);
		
	}
	strcat(buff,d);
	//strcat(buff," ");
	buff[stringlength]='\0';
	//fprintf(stderr,"0.[%s]",buff);
	size_t length = strlen(buff);
	
	if(write(writefd,&length,sizeof(size_t)) == -1){
		fprintf(stderr,"\nError Sending data.\n");
		exit(0);	
	}
	if(write(writefd,buff,length) == -1 ){
		fprintf(stderr,"\nError Sending data.\n");
		exit(0);
	}
	//printf("\n%s: %zd\n",buff,strlen(buff));
	
	free(d);
	d=NULL;
	free(buff);
	buff=NULL;
	myclose(writefd);
}

void
myread(int openClientFIFO, char *numberofwords){
	int numberWords = atoi(numberofwords),i=0;
	//fprintf(stderr,"\nNo of wrds: client :%d",numberWords);
	char *word = malloc(sizeof(char)*100);
	if(word == NULL){
		fprintf(stderr,"\nMalloc failed.\n");
		exit(0);
	}
	
	for(i=0;i<numberWords;i++){
		int wordSize=0, frequency=0;
		if(-1 == read(openClientFIFO, &wordSize, sizeof(int)) ){
			fprintf(stderr,"\nRead failed. Client.\n");
			exit(0);
			}
		if(wordSize == 0) break;
		char word[wordSize+1];
		if(-1 ==read(openClientFIFO, &word, sizeof(char)*wordSize) ){
			fprintf(stderr,"\nRead failed. Client.\n");
			exit(0);
		}
		word[wordSize+1]='\0';
		if(-1 == read(openClientFIFO, &frequency, sizeof(int)) ){
			fprintf(stderr,"\nRead failed. Client.\n");
			exit(0);
			}
		if(wordSize == 1024 && frequency == 0){
			fprintf(stderr,"\nError from Server: %s \nKilling client only.",word);
			exit(0);
		}
		fprintf(stderr,"\t%s:%d\n",word,frequency);
		
	}
		
}




void
changeDirect(char *dir){
	if ((chdir(dir)) < 0) {
		fprintf(stderr,"\nDir error.\n");
			exit(0);
		} 
}

int 
main(int argc, char **argv) {
   	validateArguments(argc,argv);
	changeDirect(argv[1]);
	if ((chdir(argv[1])) < 0) {
		fprintf(stderr,"\nDir error.\n");
			exit(0);
		} 
	createClientFIFO();
	int writefd = myopen();
	mywrite(argc, argv, writefd);
	close(writefd);
	int readfd = openClientFIFO();
	myread(readfd, argv[3]);
	close(readfd);
	char *d = malloc(sizeof(char)*8);
	snprintf(d,8,"%d",getpid());
	unlink(d);
	free(d);
	d=NULL;
    	return 0;
}
