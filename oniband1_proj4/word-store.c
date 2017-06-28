#include<stdlib.h>
#include<stdio.h>
#include<string.h>
#include <ctype.h>
#include <semaphore.h>
#include<sys/mman.h>
int newMAX=0,top_N_Words=0;
char clientpid[8]={0};
typedef struct WordNode {
	char *word;
	int frequency;
	struct WordNode* next;
}WordNode;

WordNode* search(char*);

WordNode **bucket,*locator,*result, *topfrequentwordlist;

typedef struct StopWord {
	char *word;
	struct StopWord* next;
}StopWord;
StopWord *headStopWord, *p;
void display();
void calculateMAX();
void deletelast();
void compareWordNode( WordNode *p);
void dofree();
char clientpid[8];
void
exit1(int value, char *msg){
	
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

				strcpy(bufResp,DELIMITOR);
				sem_post(responseHandler);
				sem_wait(ptr);
	shm_unlink(response_SHM_1);
	sem_close(responseHandler);
	sem_close(ptr);
			exit(0);
}

//---------------------------------------------------------------------------------STOP WORD
void createStopWordList(){
	headStopWord = malloc(sizeof(StopWord));
	if(headStopWord == NULL){
			//fprintf(stderr,"\nMem alloc problem\n");			
			exit1(0,"\nMem alloc problem\n");
			}
	headStopWord->next = NULL;

	topfrequentwordlist = malloc (sizeof(WordNode));
	if(topfrequentwordlist == NULL){
			//fprintf(stderr,"\nMem alloc problem\n");			
			exit1(0,"\nMem alloc problem\n");
			}
	topfrequentwordlist->next = NULL;
}
//--------------------------------------------------------------------------------Final Array
WordNode* createFinalArray( int top_N){
	WordNode *q = topfrequentwordlist;
	q->frequency=0;
	//q->word=malloc(sizeof(char));
	q->word=NULL;
	q->next=NULL;
	while(top_N != 1){
		q->next = malloc( sizeof (WordNode) );
		if(q->next == NULL){
			//fprintf(stderr,"\nMem alloc problem\n");			
			exit1(0,"\nMem alloc problem\n");
			}
		q=q->next;
		q->frequency=0;
		//q->word=malloc(sizeof(char));
		q->word=NULL;
		q->next=NULL;
		top_N--;	
	}
	return topfrequentwordlist;
}
//-----------------------------------------------------------------------------display Final results

void displayfinalarray(WordNode * topfrequentwordlist, int clientRDWR){
	WordNode* q = topfrequentwordlist;
	while(q != NULL){
		if(q->word != NULL){
			int wordlength=strlen(q->word)+1;
			if(-1 == write(clientRDWR,&wordlength,sizeof(int)) ){
					//fprintf(stderr,"\nWrite error to client.");
					exit1(0,"\nWrite error to client.");
				}

			if(-1 == write(clientRDWR,q->word,wordlength) ){
					//fprintf(stderr,"\nWrite error to client.");
					exit1(0,"\nWrite error to client.");
				}
			if(-1 == write(clientRDWR,&(q->frequency),sizeof(int)) ){
					//fprintf(stderr,"\nWrite error to client.");
					exit1(0,"\nWrite error to client.");
				}
		//fprintf(stderr,"\n%s:%d\n",q->word,q->frequency);
		}
		q=q->next;
	}
}
//----------------------------------------------------------------------------buckets
void createBucket(){
	bucket = malloc(10 * sizeof(WordNode*));
	if(bucket==NULL){
		//fprintf(stderr,"\nMemory allocation problem.");	
		exit1(0,"\nMemory allocation problem. ");
	}
	int i=0;
	for(i=0;i<10;i++)
	{
		bucket[i]=NULL;
	} 
	result = malloc(sizeof(WordNode *));
	if(result==NULL){
		//fprintf(stderr,"");	
		exit1(0," \nMemory allocation problem.");
	}
	result->frequency=0;
	result->next=NULL;
}
//-----------------------------------------------------------------------------------Insert in structure STOP WORDS
void insertInStopWord(char *buffer)
{
	p->next = malloc (sizeof(StopWord));
	p->next->word = malloc(sizeof(strlen(buffer)+1));
	strcpy(p->next->word,buffer);
	p->next->next=NULL;
	p=p->next;
}
//---------------------------------------------------------------------------------- Insert in structure WORDNODES
void insertInStructure(char *wordEntry)
{
	WordNode *root;						//code to isnert word on hashin function.

	StopWord *q = headStopWord;
	q=q->next;
	while(q){
		if(strcmp(q->word,wordEntry)==0)
			return;
		q=q->next;
	}
	int bucketNumber= (strlen(wordEntry)%10);
	
	locator = bucket[bucketNumber];
	while(locator != NULL) {	
		if(strcmp(locator->word, wordEntry) == 0){	//word already exits so increment frequency and exit insertfunciton
			int update = locator->frequency;
			update= update+1;
			if(newMAX<update)
				newMAX =update;
			locator->frequency = update;

			return ;
			}
		locator=locator->next;	
		}

									//word does not exists.
	locator = bucket[bucketNumber];					//wordEntry not found in the list.
		
		root = malloc(sizeof(WordNode));
		if(root == NULL){
			//fprintf(stderr,"\nError creating new node in bucket!\n");
			exit1(0," \nError creating new node in bucket!\n");
		}else{
			root->word= malloc(sizeof(strlen(wordEntry)+1));
			root->frequency=1;
			strcpy(root->word,wordEntry);
			root->next=NULL;
			if(newMAX<1)
				newMAX=1;
			if(bucket[bucketNumber] == NULL){					//wordEntry not found in the list. & list is empty. 1st entry
				bucket[bucketNumber] =root;
				}else{
				root->next=bucket[bucketNumber];
				bucket[bucketNumber]=root;
				}
		}

}

void display(int argc, char *argv[]){

	calculateMAX();

//	int clientRDWR= openClientFIFO(argv[argc]);
//	displayfinalarray(topfrequentwordlist, clientRDWR);
//	close(clientRDWR);
//	unlink(argv[argc]);
	
}
void calculateMAX(){
	int i=0;
	for(i=0;i<10;i++){
		WordNode *p = bucket[i];
		while(p){
			compareWordNode(p);
			p=p->next;
		}
	}
	
}

void compareWordNode( WordNode *p){
	
	WordNode *iter = topfrequentwordlist;
	if(iter->frequency <= p->frequency){			//1st element to be replaced. and last in linked list to be deleted.
		WordNode *newnode = malloc(sizeof(WordNode));
		if(newnode == NULL){
			//fprintf(stderr,"\n Mem alloc~\n");
			exit1(0,"\n Mem alloc~\n ");
			}
		newnode->word = p->word;
		newnode->next = iter;
		newnode->frequency = p->frequency;
		topfrequentwordlist=newnode;
		deletelast();
	}else{
	iter=iter->next;
	WordNode *iter1 = topfrequentwordlist;
	while(iter != NULL){
		if(iter->frequency <= p->frequency){
			
			WordNode *newnode = malloc(sizeof(WordNode));
			if(newnode == NULL){
				//fprintf(stderr,"\n Mem alloc~\n");
				exit1(0,"\n Mem alloc~\n ");
				}
			newnode->word = p->word;
			
			newnode->next=iter;
			iter1->next=newnode;
			newnode->frequency = p->frequency;
			deletelast();
			break;
			}
		iter=iter->next;
		iter1=iter1->next;
		}
		
	} 
	
		
}
void deletelast(){
		WordNode *iter=topfrequentwordlist;
		while(iter->next != NULL){
			
			WordNode *iter1=iter->next;
			if(iter1->next == NULL){
				iter1->next= NULL;
				iter1->word = NULL;
				free(iter1);
				iter->next = NULL;
				return;
			}
			
		iter=iter->next;
		}
}


void dofree()
{
	int i=0;
	for(i=0;i<10;i++){
	WordNode *p=bucket[i];
	WordNode *q =p;
	while(p->next!=NULL){
		p=p->next;
		q->next=NULL;
		free(q->word);
		q->word=NULL;
		free(q);
		q=p;
		}
	}
}
void newmerger(WordNode* list1, WordNode* list2){
	WordNode *p = list1;
		while(p->next){
		p=p->next;
		}
	p->next =list2;
	
}
int newlength(WordNode *root){
	int length=0;
		WordNode *p = root;
		while(p){
			length++;
			p=p->next;
			}
	
	return length;
		
}
WordNode* getnode(int max){
	
	WordNode *iter, *q=NULL;
	iter=q;
	int i=0;
	iter=malloc(sizeof(WordNode));
	for(i=0;i<10;i++){
		WordNode *p = bucket[i];
		while(p){
			if(p->frequency == max){
				iter->next=malloc(sizeof(WordNode)); 
				iter=iter->next;
				iter->word = malloc(sizeof(strlen(p->word)+1));
				if(iter->word == NULL){
						//fprintf(stderr,"\nError in malloc\n");
						exit1(0,"\n Mem alloc~\n");
					}
					if(q==NULL){		
						q=iter;
					}
					strcpy(iter->word,p->word);
				iter->frequency = p->frequency;
				iter->next=NULL;
				}
			p=p->next;
			}
		}
	if(0){
	iter=q;
	while(iter){
		//printf("\n%s %d\n",iter->word, iter->frequency);
		iter=iter->next;
		}
	}
	return q;
}
WordNode* newsort(WordNode* something){
	int len=newlength(something);
	if(len <= 0)
		return NULL;
	if(len == 1)
		return something;

	if(len > 1){
		//printf("\n%s %d",p->word,p->frequency);

	int swapped;
    	WordNode *ptr1 = something;
    	WordNode *lptr = NULL;
 	char *temp1;
    	do
    	{
        	swapped = 0;
        	ptr1 = something;
 
        	while (ptr1->next != lptr){

	            	if (strcmp(ptr1->word, ptr1->next->word) < 0){ 
	              		temp1=ptr1->next->word;
				ptr1->next->word=ptr1->word;
				ptr1->word=temp1;
	               	  	swapped = 1;
	            	}
            	ptr1 = ptr1->next;
        	}
        	lptr = ptr1;
    	}
   	 while (swapped);
	}
	return something;
}
int newgetmax(){
	int i=0, mynewMAX=0;
	for(i=0;i<10;i++){
		WordNode *p = bucket[i];
		while(p){
			if(p->frequency >=  mynewMAX){
				mynewMAX=p->frequency;
				}
			p=p->next;
			}
		}
	return mynewMAX;
}
int newgetmax2(int oldmax){
	if (oldmax == 0)
		return 0;
	int i=0, mynewMAX=0;
	for(i=0;i<10;i++){
		WordNode *p = bucket[i];
		while(p){
			if(p->frequency >  mynewMAX && p->frequency < oldmax){
				mynewMAX=p->frequency;
				}
			p=p->next;
			}
		}
	return mynewMAX;
		
}

void newprintfinal(WordNode *root, int N_Words, char *clientpid){

	//int length=0;
		//int clientRDWR= openClientFIFO(clientpid);
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

	//fprintf(stderr,"hello\n");
	//strcpy(bufResp,"hello");

		WordNode *p = root;
		while(p && N_Words){
			int wordlength=strlen(p->word)+1;
			char WordFreq[wordlength+8];
			char d[8];
			snprintf(d,8,"%d",p->frequency);
			strcpy(WordFreq, p->word);
			strcat(WordFreq, " ");
			strcat(WordFreq, d);
	
	/*			strcpy(bufResp,WordFreq);	
				sem_post(responseHandler);
				sem_wait(ptr);
	*/		if(strlen(WordFreq) > 4096){
				char arr[strlen(WordFreq)];
				strcpy(arr,WordFreq);

					int k=0;
					for(k=0; k == strlen(WordFreq); k = k+4095 ){
						strncpy(bufResp,WordFreq+k,4095);
						sem_post(responseHandler);
						sem_wait(ptr);


					}

			}else{
				strcpy(bufResp,WordFreq);
				sem_post(responseHandler);
				sem_wait(ptr);

			}
			p=p->next;
			N_Words--;
			}
				strcpy(bufResp,DELIMITOR);
				sem_post(responseHandler);
				sem_wait(ptr);
	
	p=root;
	while(p){
		WordNode *q= p;
		p=p->next;
		free(q->word);
		//free(q->frequency);
		free(q);
	}
	shm_unlink(response_SHM_1);
	sem_close(responseHandler);
	sem_close(ptr);
	//close(clientRDWR);
	//unlink(clientpid);
	
}
void newdisplay(int top_N_Words, char *clientpid){

	//printf("\n%d\n",top_N_Words);
	//printf("\n%d\n",maxfreq);
	int maxfreq;
	maxfreq=newgetmax();

	WordNode *root = newsort(getnode(maxfreq));
	while(newlength(root) < top_N_Words && maxfreq > 0){

		maxfreq=newgetmax2(maxfreq);
		WordNode *temp = getnode(maxfreq);
		temp = newsort(temp);
		newmerger(root,temp);

	}
	newprintfinal(root, top_N_Words, clientpid);
	
}

