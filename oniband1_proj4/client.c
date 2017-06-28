#include"client.h"
//#include "main.h"

void
validateArguments(int argc, char * argv[]){
	if(argc < 5){
		fprintf(stderr,"\nInvalid number of arguments:\nexpected\t \
		/word-count DIR_NAME N STOP_WORDS FILE1...\n");
		exit(0);
	}
	if(atoi(argv[2]) == 0){
		fprintf(stderr,"0 words\n");
		exit(1);
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

typedef struct {
  const char *posixName;
  int oflags;
} SemOpenArgs;

static SemOpenArgs semArgs[] = {
  { .posixName = SERVER_SEM_NAME, 
    .oflags = O_RDWR, 
  },
  { .posixName = REQUEST_SEM_NAME, 
    .oflags = O_RDWR, 
  },
 /* { .posixName = RESPONSE_SEM_NAME, 
    .oflags = O_RDWR | O_CREAT, 
  },
*/
};

static void 
semWait(sem_t *sem, const char *posixName)
{
  //SEM_VALUE("client", "before wait", sem, posixName);
  if (sem_wait(sem) < 0) {
    fprintf(stderr,"cannot wait on sem %s:", posixName);
  }
}

static void 
semPost(sem_t *sem, const char *posixName)
{
  if (sem_post(sem) < 0) {
    fprintf(stderr,"cannot post sem %s:", posixName);
  }
  //SEM_VALUE("client", "after post", sem, posixName);
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
	//close(a);

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
mywrite(int argc, char **argv, char * writefd){
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
	strcpy(writefd, buff);
	/*
	if(write(writefd,&length,sizeof(size_t)) == -1){
		fprintf(stderr,"\nError Sending data.\n");
		exit(0);	
	}
	if(write(writefd,buff,length) == -1 ){
		fprintf(stderr,"\nError Sending data.\n");
		exit(0);
	}
	*/
	//printf("\n%s: %zd\n",buff,strlen(buff));
	
	free(d);
	d=NULL;
	free(buff);
	buff=NULL;
	//myclose(writefd);
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
void
initsemaphore(){
/*	int i=0;
	for ( i = 0; i < N_SEMS; i++) {
  	  const SemOpenArgs *p = &semArgs[i];
   	 if ((sems[i] = sem_open(p->posixName, p->oflags)) == NULL) {
     	 	fprintf(stderr,"cannot open semaphore %s:", p->posixName);
    		}
  	}
*/
}
int 
main(int argc, char **argv) {
   	validateArguments(argc,argv);
	changeDirect(argv[1]);
	sem_t *sems[N_SEMS];
	//initsemaphore();
	int i=0;
	//common semaphore
	for ( i = 0; i < N_SEMS; i++) {
  	  const SemOpenArgs *p = &semArgs[i];
   	  if ((sems[i] = sem_open(p->posixName, p->oflags)) == NULL) {
     	 	fprintf(stderr,"cannot open semaphore %s:\n", p->posixName);
		exit(0);
    		}
  	}
	// common shared memory
	int fd = shm_open(SHM_NAME, O_RDWR, 0);
 		 if (fd < 0) fprintf(stderr,"cannot open shm %s:\n", SHM_NAME);
  	char *buf;
  		if ((buf = mmap(NULL, MAX_BUF, PROT_READ|PROT_WRITE, MAP_SHARED, fd, 0))== MAP_FAILED) {
    			fprintf(stderr,"cannot mmap shm %s:", SHM_NAME);
			exit(0);
  		}
  	//fprintf(stderr, "memory attached at %p\n", buf);
	char *d = malloc(sizeof(char)*8);
	snprintf(d,8,"%d",getpid());
	//fprintf(stderr,"[%s]:",d);
	
	// Response semaphore
	char response_SEM_Name_1[40];
	strcpy(response_SEM_Name_1, "/oniband1-SEM-");
	strcat(response_SEM_Name_1, d);
	//fprintf(stderr,"response_SEM_Name %s\n",response_SEM_Name);
	sem_t *ptr;
	if((ptr = sem_open(response_SEM_Name_1, O_RDWR | O_CREAT,ALL_RW_PERMS, 0 ) )== SEM_FAILED){
		fprintf(stderr,"~~~cannot open semaphore %s:%s\n", response_SEM_Name_1, strerror(errno));
		exit(0);
	}
	// Response semaphore
	char response_SEM_Name_2[40];
	strcpy(response_SEM_Name_2, "/oniband1-SEM-SECOND");
	strcat(response_SEM_Name_2, d);
	//fprintf(stderr,"response_SEM_Name %s\n",response_SEM_Name);
	sem_t *ptr1;
	if((ptr1 = sem_open(response_SEM_Name_2, O_RDWR | O_CREAT,ALL_RW_PERMS, 0 ) )== SEM_FAILED){
		fprintf(stderr,"~~~cannot open semaphore %s:%s\n", response_SEM_Name_2, strerror(errno));
		exit(0);
	}
	// Response shared memory
	char response_SHM[40];
	strcpy(response_SHM, "/oniband1-");
	strcat(response_SHM, d);
	//fprintf(stderr,"response_SHM %s\n",response_SHM);
	int response = shm_open(response_SHM, O_RDWR | O_CREAT, ALL_RW_PERMS);
	if(response < 0){
		fprintf(stderr,"cannot open shared memory %s:\n", d);
		exit(0);
	}
	if (ftruncate(response, MAX_BUF) < 0) {
   		 fprintf(stderr,"cannot size shm %s to %d:", SHM_NAME, MAX_BUF);
  	}
	char *bufResp;
  		if ((bufResp = mmap(NULL, MAX_BUF, PROT_READ|PROT_WRITE, MAP_SHARED, response, 0))== MAP_FAILED) {
    			fprintf(stderr,"cannot mmap shm %s:", d);
			exit(0);
  		}
  	//fprintf(stderr, "Response memory attached at %p\n", bufResp);

	semWait(sems[SERVER_SEM], SERVER_SEM_NAME);
	mywrite(argc, argv, buf);
    	semPost(sems[REQUEST_SEM], REQUEST_SEM_NAME);
    	semPost(sems[SERVER_SEM], SERVER_SEM_NAME);
	
/*	
	for(i=0; i< atoi(argv[2])  ; i++){
	//fprintf(stderr,"\nIn here");
			fprintf(stderr,"\n");
    		semWait(ptr, response_SEM_Name_1);
			fprintf(stderr,"%s",bufResp);
		semPost(ptr1,response_SEM_Name_2);
	
	}
*/
	while(1){
		semWait(ptr, response_SEM_Name_1);
		if(strcmp(bufResp,DELIMITOR)==0){
			semPost(ptr1,response_SEM_Name_2);
			break;
		}
		else{	fprintf(stderr,"\n%s",bufResp);}
		semPost(ptr1,response_SEM_Name_2);
	
	}
	sem_close(ptr);
	sem_close(ptr1);
	shm_unlink(response_SEM_Name_1);
	shm_unlink(response_SHM);
	
	
	
    	return 0;
}
