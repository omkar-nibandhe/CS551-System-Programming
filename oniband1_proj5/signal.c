

void
term(){

  	//fprintf(stderr,"\nSIGTERM CAUGHT\n");
	//sigaction(SIG_IGN, &action, NULL);
	errorlogentry("SIGTERM DETECTED\n");
	serverstop();
	pthread_exit(0);
	//pthread_kill(0);
//	exit1(0,"SIGTERM caught\n");
}
