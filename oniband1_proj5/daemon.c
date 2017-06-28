#include "lock.c"

void
makeDaemon(char * dir_name){
	 pid_t pid=0,sid=0;
   	 pid = fork ( );  
   		 if (pid < 0){  
       			fprintf(stderr,"\nFork failed.\n");
			exit(0);
		}
    		 if (pid > 0)  {
			fprintf(stderr,"\nDaemon Pid:[%d]\n",pid);
        		exit(0);  
	
		}
		
	umask(0);
	sid=setsid();	
		if(sid < 0){
        		fprintf(stderr,"\nSession failed.\n");
			exit(0);
		}
	if(lockPidFile() != 0){
		fprintf(stderr,"\nLocking Session failed.\n");
		exit(1);

	}
    	if ((chdir (".")) < 0) {
		fprintf(stderr,"\nDir error.\n");
			exit(0);
		} 
}
