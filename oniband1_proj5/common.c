#define DELIMITOR "|"
//#include "server_help.c"
void
validateArgumentClient(int argc, char * argv[]){
	if(argc < 6){
		fprintf(stderr,"\nInvalid number of arguments:\nexpected\t \
		/word-count HOSTNAME PORT N STOP_WORDS FILE1...\n");
		exit(0);
	}
	if(atoi(argv[3]) == 0){
		//fprintf(stderr,"0 words\n");
		exit(1);
	}
	
}
void
validateArguments(int argc){
	if(argc != 3){
		fprintf(stderr,"\nInvalid number of arguments:\nexpected\t./word-countd DIR_NAME PORT\n");
		exit(0);
	}
	
}

void
changeDirect(char *dir){
	if ((chdir(dir)) < 0) {
		fprintf(stderr,"\nDir error.\n");
			exit(0);
		} 
}


