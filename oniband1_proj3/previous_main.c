#include<stdio.h>
#include<stdlib.h>
#include<ctype.h>
#include<string.h>
#include <dlfcn.h>
#include <gnu/lib-names.h>
typedef int (IsWordChar)(int c);

#define IS_WORD_CHAR "isWordChar"
#include "word-store.c"

int previous_main(int argc, char **argv) {
	strcpy(clientpid ,(argv[argc-1]) );	
	if(argc < 4) {
		exit1(0,"Command line arguments inappropriate.\tExiting program.\n");
	}
	top_N_Words = atoi(argv[1]);
	if(top_N_Words == 0){
		//fprintf(stderr,"\n 0 words to print.\n %s\n",argv[argc-1]);
		int clientRDWR= openClientFIFO(argv[argc-1]);
		
		int wordlength=0;
			if(-1 == write(clientRDWR,&wordlength,sizeof(int)) ){
					
					exit1(0,"\nWrite error to client.");
				}
			if(-1 == write(clientRDWR,"",wordlength) ){
					exit1(0,"\nWrite error to client.");
				}
			if(-1 == write(clientRDWR,&wordlength,sizeof(int)) ){
					exit1(0,"\nWrite error to client.");
				}
		close(clientRDWR);
		unlink(argv[argc-1]);
		exit(0);
	}
	//fprintf(stderr,"\n%s mod name\n",argv[0]);
	char *modName = argv[0];
	void *dlHandle = dlopen(modName, RTLD_NOW);
 		 if (dlHandle == NULL) {
   			 fprintf(stderr, "cannot open dynamic module %s: %s\n", modName, dlerror());
			 exit1(0, "cannot open dynamic module");
   			 exit(0);
  		}
	IsWordChar *isWordChar = dlsym(dlHandle, IS_WORD_CHAR);
 		 if (isWordChar == NULL) {
   			 fprintf(stderr, "cannot resolve %s in %s: %s\n", IS_WORD_CHAR, modName,dlerror());
			  exit1(0, "cannot resolve dynamic module");
    			exit(1);
 		 }
	
	int fileCount=0;
	createStopWordList();
	topfrequentwordlist = createFinalArray(top_N_Words);
	
	
	
	p=headStopWord;
	//----------------------------------------------------------
	//read stop words
	FILE *stop_word_file = fopen(argv[2], "r");
	if(stop_word_file == NULL) {
		//fprintf(stderr,"\n Unable to open Stop-Word file.\nExiting program.\n");
		exit1(0,"\n Unable to open Stop-Word file.\nExiting program.\n");
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
					exit1(0,"\n Memory realloc problem.\n");
					
				}else{
				strcpy(newbuffer,buffer);
				free(buffer);
				buffer=newbuffer;
				}
    			}
		int c = (int)stopWordChar;
		//if(stopWordChar == '\'' || isalnum(stopWordChar) ) {
		if(isWordChar( c )){
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
	for(fileCount=3; fileCount<argc-1; fileCount++) {
		//printf("\nfileCount=>%d: %s\n",fileCount,argv[fileCount]);
		FILE *fp = fopen(argv[fileCount], "r");
		if(fp == NULL) {
			fprintf(stderr,"Unable to open input file: %s.\n Press key to continue.\n", argv[fileCount]);
			//getchar();
			exit1(0,"Unable to open input file:");
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
					exit1(0,"\n Memory realloc problem.\n");
				}
				else{
				strcpy(newbuffer,buffer);
				free(buffer);
				buffer=newbuffer;
				}
    			}
		//int c = (int)ch;
		//if(stopWordChar == '\'' || isalnum(stopWordChar) ) {
		if(isWordChar( ch )){
			buffer[length++]=tolower(ch);
			buffer[length]='\0';
			count++;

			}else{
				if(length){
				//printf("\n-%zd %s\n",strlen(buffer),buffer);
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
	
	
	
	//display(argc-1, argv);
	//dofree();
	if (dlclose(dlHandle) != 0) {
    		fprintf(stderr, "cannot close module %s: %s\n", modName, dlerror());
		
    		exit1(0,"Closing module error.");
  	}
	newdisplay(top_N_Words, argv[argc-1]);	
	return 0;

}

