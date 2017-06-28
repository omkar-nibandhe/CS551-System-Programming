
/*

void
errorlog(char * errormsg, char *argv){



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
           fprintf(ptr,"\n%s|%s|%s", outstr,errormsg,argv);

		fclose(ptr);

}
*/
