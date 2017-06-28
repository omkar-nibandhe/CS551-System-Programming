#include<stdio.h>
#include<stdlib.h>
#include<ctype.h>
#include<string.h>
#include <dlfcn.h>
#include <gnu/lib-names.h>
#include "word-store.c"

int previous_main(int argc, char **argv) {
	int iiii=0;
	while(argv[iiii]!=NULL){
			//fprintf(stderr,"\nPrevious main:%d:arg %s",iiii,argv[iiii]);
			iiii++;
		}
	//strcpy(clientpid ,(argv[argc-1]) );	
	if(argc < 2) {
		//exit1(0,"Command line arguments inappropriate.\tExiting program.\n");
		exit(0);
	}
	top_N_Words = atoi(argv[0]);
	//fprintf(stderr,"top_N_Words: %d\n",top_N_Words);

	if(top_N_Words == 0){
			// Response shared memory
	char response_SHM_1[40];
	strcpy(response_SHM_1, "/oniband1-");
	strcat(response_SHM_1, clientpid);
	int response = shm_open(response_SHM_1, O_RDWR, 0);
	if ( response < 0 ){
		fprintf(stderr,"cannot open shared memory %s:\n", response_SHM_1);
		exit(0);
	}
	// Response semaphore
	char response_SEM_Name_2[40];
	strcpy(response_SEM_Name_2, "/oniband1-SEM-SECOND");
	strcat(response_SEM_Name_2, clientpid);
	//fprintf(stderr,"response_SEM_Name %s\n",response_SEM_Name);
	sem_t *ptr;
	if((ptr = sem_open(response_SEM_Name_2, O_RDWR | O_CREAT, ALL_RW_PERMS, 0 ) )== SEM_FAILED){
		fprintf(stderr,"~~~cannot open semaphore %s:%s\n", response_SEM_Name_2, strerror(errno));
		exit(0);
	}
	
	char *bufResp;
  		if ((bufResp = mmap(NULL, MAX_BUF, PROT_READ|PROT_WRITE, MAP_SHARED, response, 0))== MAP_FAILED) {
    			fprintf(stderr,"cannot mmap shm %s:", clientpid);
			exit(0);
  		}
  	//fprintf(stderr, "Response memory attached at %p\n", bufResp);
	// Response semaphore
	char response_SEM_Name[40];
	strcpy(response_SEM_Name, "/oniband1-SEM-");
	strcat(response_SEM_Name,clientpid);
	sem_t *responseHandler;
	if((responseHandler = sem_open(response_SEM_Name, O_RDWR ) ) == NULL){
		fprintf(stderr,"!!cannot open semaphore %s: %s\n", response_SEM_Name,strerror(errno));
		exit(0);
	}
	shm_unlink(response_SHM_1);
	shm_unlink(response_SEM_Name_2);
	sem_close(ptr);
	sem_close(responseHandler);
	}
	//fprintf(stderr,"\n%s mod name\n",argv[0]);

	int fileCount=0;
	createStopWordList();
	topfrequentwordlist = createFinalArray(top_N_Words);
	
	
	
	p=headStopWord;
	//----------------------------------------------------------
	//read stop words
	FILE *stop_word_file = fopen(argv[1], "r");
	if(stop_word_file == NULL) {
		//fprintf(stderr,"\n Unable to open Stop-Word file.\nExiting program.\n");
		//exit1(0,"\n Unable to open Stop-Word file.\nExiting program.\n");
		exit(0);
	}
	else{
		
		
		int CUR_MAX = 4095; //4095
		char *buffer = (char*) malloc(sizeof(char) * CUR_MAX); // allocate buffer.
		int count = 0; 
		int length = 0;
		char stopWordChar = tolower(getc(stop_word_file));
		
		while( (stopWordChar != EOF)   ){
   		if((count+1) == CUR_MAX) { // time to expand ?
      			CUR_MAX *= 2; // expand to double the current size of anything similar.
      			count = 0;	
      			char* newbuffer = realloc(buffer, CUR_MAX); // re allocate memory. 
				if(buffer == NULL){
					fprintf(stderr,"\n Memory realloc problem.\n");
					//exit1(0,"\n Memory realloc problem.\n");
					exit(0);
					
				}else{
				strcpy(newbuffer,buffer);
				free(buffer);
				buffer=newbuffer;
				}
    			}
		//int c = (int)stopWordChar;
		if(stopWordChar == '\'' || isalnum(stopWordChar) ) {
		//if(isWordChar( c )){
			buffer[length++]=tolower(stopWordChar);
			buffer[length]='\0';
			count++;

			}else{
				if(length){
				//printf("\nStop-word:-%zd %s\n",strlen(buffer),buffer);
				insertInStopWord(buffer);
				
				count=length=0;
				CUR_MAX = 4095;
				buffer[length]='\0';
				}
			}
    			stopWordChar = tolower(getc(stop_word_file)); // read from stream.
    			
		}if(length){
				//printf("\nStop-word:-%zd %s\n",strlen(buffer),buffer);
				insertInStopWord(buffer);
				
				count=length=0;
				CUR_MAX = 4095;
				buffer[length]='\0';
				}
		
	
	
	}
	fclose(stop_word_file);
	//getchar();
	//----------------------------------------------------------
	createBucket();
	
		
	int ch;
	int CUR_MAX = 4095; //4095
	char *buffer = (char*) malloc(sizeof(char) * CUR_MAX); // allocate buffer.
	int count = 0; 
	int length = 0;	
	//fprintf(stderr,"\n%d:filecount, %d : argc",fileCount, argc);
	for(fileCount=2; fileCount<argc; fileCount++) {
		//printf("\nfileCount=>%d: %s\n",fileCount,argv[fileCount]);
		FILE *fp = fopen(argv[fileCount], "r");
		if(fp == NULL) {
			fprintf(stderr,"Unable to open input file: %s.\n Press key to continue.\n", argv[fileCount]);
			exit(0);
			}
		ch=getc(fp);
		while( (ch != EOF)   ){
   		if((count+1) == CUR_MAX) { // time to expand ?
      			CUR_MAX *= 2; // expand to double the current size of anything similar.
      			count = 0;	
      			char* newbuffer = realloc(buffer, CUR_MAX); // re allocate memory. 
				if(buffer == NULL){
					//fprintf(stderr,"\n Memory realloc problem.\n");
					//exit1(0,"\n Memory realloc problem.\n");
					exit(0);
				}
				else{
				strcpy(newbuffer,buffer);
				free(buffer);
				buffer=newbuffer;
				}
    			}
		
		if(ch == '\'' || isalnum(ch) ) {
		
			buffer[length++]=tolower(ch);
			buffer[length]='\0';
			count++;

			}else{
				if(length){
				//fprintf(stderr,"\n-%zd %s\n",strlen(buffer),buffer);
				insertInStructure(buffer);
				count=length=0;
				CUR_MAX = 4095;
				buffer[length]='\0';
				}
			}
    			ch = tolower(getc(fp)); // read from stream.
    			
		}
		if(length){
				//printf("\n-%zd %s\n",strlen(buffer),buffer);
				insertInStructure(buffer);
				count=length=0;
				CUR_MAX = 4095;
				buffer[length]='\0';
				}
		
		
		fclose(fp);
		

	}
	
	newdisplay(top_N_Words, argv[argc]);	

	return 0;

}

