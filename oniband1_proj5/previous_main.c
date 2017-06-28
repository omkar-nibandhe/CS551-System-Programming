#include<stdio.h>
#include<stdlib.h>
#include<ctype.h>
#include<string.h>
#include <dlfcn.h>
#include <gnu/lib-names.h>

//typedef int (IsWordChar)(int c);

//#define IS_WORD_CHAR "isWordChar"
#include "word-store.c"

int 
openClientFIFO(char * arg){
	return atoi(arg);
}
void
validateArg(int argc, char **argv)
{

	int i=2;
	char *fname =argv[i];
	while(i != (argc-2)){
		if( access( fname, F_OK | R_OK ) != -1 ) {
			// file exists so skip to next file
			fname=argv[++i];
	
		} else {
    			// file doesn't exist
			
			//errorlogentry("unable to open file");
			exit1(0,"unable to open file.");
		}
	}
}
int previous_main(int argc, char **argv) {
	int writefd = atoi(argv[argc-1]);
	strcpy(clientpid ,(argv[argc-1]) );	
	validateArg(argc,argv);
	int top_N_Words=0;
	WordNode *result, *locator, **bucket;
	//fprintf(stderr,"\n %d writefd",writefd);
	int i=0;
	//for(i=0; i< argc; i++)
	//	fprintf(stderr,"arg[%d]=%s\n",i,argv[i]);

	if(argc < 4) {
		exit1(0,"Command line arguments inappropriate.\tExiting program.\n");
	}
	top_N_Words = atoi(argv[0]);
	if(top_N_Words == 0){
		//fprintf(stderr,"\n 0 words to print.\n %s\n",argv[argc-1]);
		int clientRDWR= openClientFIFO(argv[argc-1]);
		
		int wordlength=0;
			if(-1 == write(clientRDWR,&wordlength,sizeof(int)) ){
					
					exit1(0,"\nWrite error to client.\n");
				}
			if(-1 == write(clientRDWR,"",wordlength) ){
					exit1(0,"\nWrite error to client.\n");
				}
			if(-1 == write(clientRDWR,&wordlength,sizeof(int)) ){
					exit1(0,"\nWrite error to client.\n");
				}
		close(clientRDWR);
		//unlink(argv[argc-1]);
		//return 0;
		//pthread_exit(0);
	}
	
	int fileCount=0;
	StopWord *headStopWord;
	headStopWord = createStopWordList(headStopWord);
	WordNode * topfrequentwordlist;
	topfrequentwordlist = createtopfrequentwordlist(topfrequentwordlist);
	//fprintf(stderr,"main %p\n",headStopWord);
	topfrequentwordlist = createFinalArray(top_N_Words, topfrequentwordlist);
	
	
	
	//p=headStopWord;
	//----------------------------------------------------------
	//read stop words
	FILE *stop_word_file = fopen(argv[1], "r");
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
		if(stopWordChar == '\'' || isalnum(stopWordChar) ) {
		//if(isWordChar( c )){
			buffer[length++]=tolower(stopWordChar);
			buffer[length]='\0';
			count++;

			}else{
				if(length){
				//printf("\nStop-word:-%zd %s\n",strlen(buffer),buffer);
				insertInStopWord(buffer, headStopWord);
				
				count=length=0;
				CUR_MAX = 4095;
				buffer[length]='\0';
				}
			}
    			stopWordChar = tolower(getc(stop_word_file)); // read from stream.
    			
		}if(length){
				//printf("\nStop-word:-%zd %s\n",strlen(buffer),buffer);
				insertInStopWord(buffer, headStopWord);
				
				count=length=0;
				CUR_MAX = 4095;
				buffer[length]='\0';
				}
		
	
	
	}
	fclose(stop_word_file);
	//getchar();
	//----------------------------------------------------------
	bucket = createBucket(result,bucket);
	
		
	int ch;
	int CUR_MAX = 4095; //4095
	char *buffer = (char*) malloc(sizeof(char) * CUR_MAX); // allocate buffer.
	int count = 0; 
	int length = 0;	
	for(fileCount=2; fileCount<argc-2; fileCount++) {
		//printf("\nfileCount=>%d: %s\n",fileCount,argv[fileCount]);
		FILE *fp = fopen(argv[fileCount], "r");
		if(fp == NULL) {
			fprintf(stderr,"Unable to open input file: %s.\n Press key to continue.\n", argv[fileCount]);
			//getchar();
			exit1(0,"Unable to open input file\n");
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
		if(ch == '\'' || isalnum(ch) ) {
		//if(isWordChar( ch )){
			buffer[length++]=tolower(ch);
			buffer[length]='\0';
			count++;

			}else{
				if(length){
				//printf("\n-%zd %s\n",strlen(buffer),buffer);
				insertInStructure(buffer,locator, headStopWord, bucket);
				count=length=0;
				CUR_MAX = 4095;
				buffer[length]='\0';
				}
			}
    			ch = tolower(getc(fp)); // read from stream.
    			
		}
		if(length){
				//printf("\n-%zd %s\n",strlen(buffer),buffer);
				insertInStructure(buffer, locator, headStopWord, bucket);
				count=length=0;
				CUR_MAX = 4095;
				buffer[length]='\0';
				}
		
		
		fclose(fp);
		

	}
	
	
	
	//display(argc-1, argv);
	//dofree();
	/*if (dlclose(dlHandle) != 0) {
    		fprintf(stderr, "cannot close module %s: %s\n", modName, dlerror());
		
    		exit1(0,"Closing module error.");
  	}
	*/
	newdisplay(top_N_Words, argv[argc-1], bucket,writefd);	
	dofree(bucket);
	return 0;

}

