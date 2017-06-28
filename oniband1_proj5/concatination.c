

void
mywrite(int argc, char **argv, int writefd){
	int i=0;
	size_t stringlength=0;
	
	for(i=3;i<argc;i++){
		stringlength += strlen(argv[i]);
		}
	//printf("\n%zd\n",stringlength);
	stringlength = (stringlength+argc+8);
	char *buff= malloc(stringlength*sizeof(char));
	char *d = malloc(sizeof(char)*8);
	snprintf(d,8,"%d",getpid());
	for(i=3; i < argc; i++){
		strcat( buff,argv[i] );
		strcat( buff, DELIMITOR );
	}
	strcat(buff,d);
	//strcat(buff," ");
	buff[stringlength]='\0';
	//fprintf(stderr,"0.[%s]",buff);
	size_t length = strlen(buff);
	
	//if(write(writefd, &length, sizeof(int)) == -1){
	//	fprintf(stderr,"\nError Sending data.\n");
	//	exit(0);	
	//}
	if(write(writefd, buff, length) == -1 ){
		fprintf(stderr,"\nError Sending data.\n");
		exit(0);
	}
	//printf("\nClient%s: %zd\n",buff,strlen(buff));
	
	free(d);
	d=NULL;
	free(buff);
	buff=NULL;
	//myclose(writefd);
}

void
myread(int numberWords, int socketID){

	////fprintf(stderr,"%d socketID %d numberwords\n",socketID, numberWords);
	
	int i = 0;
	for(i=0;i<numberWords;i++){
		int wordSize=0, frequency=0;
		/*

		read(socketID, &wordSize, sizeof(int));
		char word[wordSize+1];
		word[wordSize+1]='\0';
		read(socketID, &word, sizeof(char)*wordSize);
		read(socketID, &frequency, sizeof(int));
	*/
		if(-1 == read(socketID, &wordSize, sizeof(int)) ){
			fprintf(stderr,"\nRead failed. Client.\n");
			exit(0);
			}
		if(wordSize == 0) break;
		char word[wordSize+1];
		word[wordSize+1]='\0';
		if(-1 ==read(socketID, &word, sizeof(char)*wordSize) ){
			fprintf(stderr,"\nRead failed. Client.\n");
			exit(0);
		}
		if(-1 == read(socketID, &frequency, sizeof(int)) ){
			fprintf(stderr,"\nRead failed. Client.\n");
			exit(0);
			}
		if(wordSize == 1024 && frequency == 0){
			fprintf(stderr,"\nError from Server: %s\n",word);
			exit(0);
		}
		fprintf(stderr,"\t%s:%d\n",word,frequency);
		
	}
}
