void
createlogfile(){
	FILE *requestlog = fopen(REQLOG,"a");
	FILE *errorslog = fopen(ERRLOG,"a");
	if(requestlog == NULL || errorslog == NULL){
		fprintf(stderr,"Error opening log files\n");
		exit(1);
	}
	fclose(requestlog);
	fclose(errorslog);
}

void
serverstart(){
	 time_t t;
	 struct tm *tmp;
	char outstr[200];
           t = time(NULL);
           tmp = localtime(&t);
           if (tmp == NULL) {
               perror("localtime");
               exit(EXIT_FAILURE);
           }

           if (strftime(outstr, 200 ,"%Y-%m-%d-%H:%M:%S", tmp) == 0) {
               fprintf(stderr, "strftime returned 0");
               exit(EXIT_FAILURE);
           }

		FILE *ptr;
		ptr= fopen(ERRLOG,"a");
           fprintf(ptr,"%s|SERVER_START", outstr);
		fclose(ptr);
          // exit(EXIT_SUCCESS);

}

void
serverstop(){
	 time_t t;
	 struct tm *tmp;
	 char outstr[200];
           t = time(NULL);
           tmp = localtime(&t);
           if (tmp == NULL) {
               perror("localtime");
               exit(EXIT_FAILURE);
           }

           if (strftime(outstr, 200 ,"%Y-%m-%d%H:%M:%S", tmp) == 0) {
               fprintf(stderr, "strftime returned 0");
               exit(EXIT_FAILURE);
           }

		FILE *ptr;
		ptr= fopen(ERRLOG,"a");
           fprintf(ptr,"%s|SERVER_STOP", outstr);
		fclose(ptr);
          // exit(EXIT_SUCCESS);
}

void
logentry(char** arg, int n){

	//fprintf(stderr,"Argument: %s \n",(char*)arg);
	 time_t t;
	 struct tm *tmp;
	 char outstr[200];
           t = time(NULL);
           tmp = localtime(&t);
           if (tmp == NULL) {
               perror("localtime");
               exit(EXIT_FAILURE);
           }

           if (strftime(outstr, 200 ,"%Y-%m-%d%H:%M:%S", tmp) == 0) {
               fprintf(stderr, "strftime returned 0");
               exit(EXIT_FAILURE);
           }

		FILE *ptr;
		ptr= fopen(REQLOG,"a");
           fprintf(ptr,"\n%s", outstr);
		int i =0 ;
		for(i=0 ; i < n ; i++){
			fprintf(ptr,"|%s",arg[i]);
		}
		fclose(ptr);
}

void
logentryend(char **arg,int n,int diff){

	 time_t t;
	 struct tm *tmp;
	 char outstr[200];
           t = time(NULL);
           tmp = localtime(&t);
           if (tmp == NULL) {
               perror("localtime");
               exit(EXIT_FAILURE);
           }

           if (strftime(outstr, 200 ,"%Y-%m-%d%H:%M:%S", tmp) == 0) {
               fprintf(stderr, "strftime returned 0");
               exit(EXIT_FAILURE);
           }

		FILE *ptr;
		ptr= fopen(REQLOG,"a");
           fprintf(ptr,"\n%s", outstr);
		int i =0 ;
		for(i=0 ; i < n-1 ; i++){
			fprintf(ptr,"|%s",arg[i]);
		}

	fprintf(ptr,"|%d",diff);
		fclose(ptr);

}

void
errorlogentry( char * errormsg){

	//fprintf(stderr,"Argument: %s \n",(char*)arg);
	 time_t t;
	 struct tm *tmp;
	 char outstr[200];
           t = time(NULL);
           tmp = localtime(&t);
           if (tmp == NULL) {
               perror("localtime");
               exit(EXIT_FAILURE);
           }

           if (strftime(outstr, 200 ,"%Y-%m-%d%H:%M:%S", tmp) == 0) {
               fprintf(stderr, "strftime returned 0");
               exit(EXIT_FAILURE);
           }

		FILE *ptr;
		ptr= fopen(ERRLOG,"a");
           fprintf(ptr,"\n%s|%s", outstr,errormsg);

		fclose(ptr);
//	pthread_exit(0);
}
