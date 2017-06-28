#include<stdio.h>
#include<stdlib.h>
#include<ctype.h>
#include<string.h>
#include "word-store.h"
#include "word-store.c"
#include<unistd.h>
#include<sys/types.h>



int main(int argc, char **argv) {
	
	if(argc < 4) {
		fprintf(stderr,"\nCommand line arguments inappropriate.\tExiting program.\n");
		fprintf(stderr,"\tExpected:  ./word-count N STOP_WORD filename1.txt filename2.txt\n");
		exit(0);
	}

	
	top_N_Words =atoi( argv[1]);
	
	if(top_N_Words <= 0){
		fprintf(stderr,"\n No words to print.\n");
		exit(0);
	}
	int fileCount=0, i=3;
	createStopWordList();
	topfrequentwordlist = createFinalArray(top_N_Words);
	
	char *fname =argv[i];
	while(i != argc){
		if( access( fname, F_OK | R_OK ) != -1 ) {
			// file exists so skip to next file
			fname=argv[++i];
	
		} else {
    			// file doesn't exist
			fprintf(stderr,"Some files missing.\nExiting.\n");
			exit(0);
		}
	}
	p=headStopWord;
	//----------------------------------------------------------
	//read stop words
	FILE *stop_word_file = fopen(argv[2], "r");
	if(stop_word_file == NULL) {
		fprintf(stderr,"\n Unable to open Stop-Word file.\nExiting program.\n");
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
					exit(0);
				}else{
				strcpy(newbuffer,buffer);
				//free(buffer);
				buffer=newbuffer;
				}
    			}
		if(stopWordChar == '\'' || isalnum(stopWordChar) ) {
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
	//pid_t pid[argc-3];
	int arrayPipes[argc-3][2];
	for(i=0;i<argc-3;i++){
		if(pipe(arrayPipes[i])<0)
	  	{	fprintf(stderr,"\nError in creating pipe.\n");
			exit(-1);
		}	
	}
	
	for(i=0;i<argc-3;i++){
		//pid[i]=fork();
			//fileCount=3;
		pid_t pid=fork();
		if(pid<0){
			fprintf(stderr,"\nError forking\nExiting\n");
			exit(-1);
		}
		else if(0==pid){				//child will handle this.
			//printf("-->%d",i);
			close(arrayPipes[i][0]);
			//printf("\npid[%d]:%d parent is: %d\n",i,(int)pid[i],(int)getpid());
			FILE *fp = fopen(argv[i+3], "r");
			if(fp == NULL) {
				fprintf(stderr,"Unable to open input file: %s.\n Press key to continue.\n", argv[fileCount]);
				getchar();
				exit(0);
				}
			WordNode *head = malloc(sizeof(WordNode));
			head->next = NULL;
			char ch;
			int CUR_MAX =4096; //4095
			char *buffer = malloc(sizeof(char) * CUR_MAX); // allocate buffer.
			int count = 0; 
			int length = 0;	
			int listCounter=0;
			ch=getc(fp);
			while( (ch != EOF)   ){
   				if((count+1) == CUR_MAX) { 		// time to expand ?
      					CUR_MAX = CUR_MAX +1000; 		// expand to double the current size of anything similar.
      					count = 0;	
      					buffer = realloc(buffer, CUR_MAX); // re allocate memory. 
					if(buffer == NULL){
						fprintf(stderr,"\n Memory realloc problem.\n");
						exit(0);
					}
					//strcpy(newbuffer,buffer);
					//free(buffer);
					//buffer=newbuffer;
					
				} /*
				else{
					strncpy(newbuffer,buffer,CUR_MAX);
					free(buffer);
					buffer=newbuffer;
					} */ 
    				//}
				if(ch == '\'' || isalnum(ch) ) {
				buffer[length++]=tolower(ch);
				buffer[length]='\0';
				count++;
				
				}else{
					if(length){
					//printf("\n-%zd\n",strlen(buffer));
					mylistinsert(buffer,head);
					//listCounter++;
					count=length=0;
					CUR_MAX = 65535;
					buffer[length]='\0';
					}
				}

    			ch = tolower(getc(fp)); // read from stream.
			}
			if(length){
				//printf("\n-%d\n",(int)strlen(buffer));
				mylistinsert(buffer, head);
				//listCounter++;
				count=length=0;
				CUR_MAX = 409600;
				buffer[length]='\0';
				}
			// make child work here.
			//displayChildList(head);
			listCounter=counterlist(head);
			//printf("\n%d total words from child %d.\n",listCounter,i);
			write(arrayPipes[i][1], &listCounter, sizeof(int));
			WordNode *ptr= head->next;
			while(ptr != NULL ){
				//printf("\n \t freq-->%d.\n",ptr->frequency);
				int wordlength= strlen(ptr->word);
				write(arrayPipes[i][1], &(wordlength),sizeof(int));
				write(arrayPipes[i][1], (ptr->word), wordlength);
				write(arrayPipes[i][1], &(ptr->frequency), sizeof(int));
				ptr=ptr->next;
				
			}
			close(arrayPipes[i][1]);
			fclose(fp);

			//pipe send to parent. DATA 3 part
			exit(0);	
		}
		//fileCount++;
	}	
	//printf("parent is here:\n");	
	//printf("\n");
	

	for(i=0;i<argc-3;i++){
		//int status;
		//wait(status);	
		close(arrayPipes[i][1]);
		int listCounterParent;
		read(arrayPipes[i][0],&listCounterParent,sizeof(int));
		//printf("\n\n-----------------------------------------------------------------------\nNumber of wordNodes -> %d",listCounterParent);
		int wordlengthParent=0,freq=0;
		while(listCounterParent){
			read(arrayPipes[i][0],&wordlengthParent,sizeof(int));
				char tempWord[wordlengthParent + 1];
				WordNode *reader=malloc(sizeof(WordNode)*sizeof(char));
				reader->word=malloc(sizeof(char)*wordlengthParent);
			read(arrayPipes[i][0], tempWord, wordlengthParent);
				tempWord[wordlengthParent] = '\0';
				strcpy(reader->word,tempWord);
			read(arrayPipes[i][0],&freq,sizeof(int));
				reader->frequency=(int)freq;
			
			insertInStructure(reader->word,reader->frequency);
			//printf("\nWord %s Count %d",reader->word,reader->frequency);
			//free(reader->word);
			//free(reader);
			--listCounterParent;
		}
	}
	//display();
	newdisplay(top_N_Words);
	//dofree();	
	return 0;

}

