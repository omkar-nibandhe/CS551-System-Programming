#include<stdlib.h>
#include<stdio.h>
#include<string.h>
#include <ctype.h>
//int newMAX=0;
typedef struct WordNode {
	char *word;
	int frequency;
	struct WordNode* next;
}WordNode;

WordNode* search(char*);

//WordNode *topfrequentwordlist;

typedef struct StopWord {
	char *word;
	struct StopWord* next;
}StopWord;
char clientpid[8]={0};

void display(int argc, char *argv[], WordNode **bucket, WordNode *topfrequentwordlist);
void calculateMAX(WordNode **bucket, WordNode *topfrequentwordlist);
void deletelast(WordNode *topfrequentwordlist);
void compareWordNode( WordNode *p, WordNode *topfrequentwordlist);
void dofree(WordNode **bucket);

void
exit1(int value, char *msg){
	errorlogentry(msg);
	int clientRDWR= openClientFIFO(clientpid);
	int wordlength=strlen(msg)+1;
	int errorcode = 1024;
			if(-1 == write(clientRDWR,&errorcode,sizeof(int)) ){
					//fprintf(stderr,"\nWrite error to client.");
					pthread_exit(0);
				}

			if(-1 == write(clientRDWR,msg,wordlength) ){
					//fprintf(stderr,"\nWrite error to client.");
					pthread_exit(0);
				}
			if(-1 == write(clientRDWR,&value,sizeof(int)) ){
					//fprintf(stderr,"\nWrite error to client.");
					pthread_exit(0);
				}
	close(clientRDWR);
	unlink(clientpid);
	pthread_exit(0);
	
}

//---------------------------------------------------------------------------------STOP WORD
StopWord *
createStopWordList(StopWord *headStopWord){
	//StopWord *headStopWord;
	headStopWord = malloc(sizeof(StopWord));
	if(headStopWord == NULL){
			//fprintf(stderr,"\nMem alloc problem\n");			
			exit1(0,"\nMem alloc problem\n");
			}
	//fprintf(stderr,"%p\n",headStopWord);
	headStopWord->next = NULL;
	/*
	topfrequentwordlist = malloc (sizeof(WordNode));
	if(topfrequentwordlist == NULL){
			//fprintf(stderr,"\nMem alloc problem\n");			
			exit1(0,"\nMem alloc problem\n");
			}
	topfrequentwordlist->next = NULL;
	*/
	return headStopWord;
}
WordNode *
createtopfrequentwordlist( WordNode * topfrequentwordlist){
	topfrequentwordlist = malloc (sizeof(WordNode));
	if(topfrequentwordlist == NULL){
			//fprintf(stderr,"\nMem alloc problem\n");			
			exit1(0,"\nMem alloc problem\n");
			}
	topfrequentwordlist->next = NULL;
	return topfrequentwordlist;
}
	
//--------------------------------------------------------------------------------Final Array
WordNode* createFinalArray( int top_N, WordNode *topfrequentwordlist){
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
					exit1(0,"\nWrite error to client.\n");
				}

			if(-1 == write(clientRDWR,q->word,wordlength) ){
					//fprintf(stderr,"\nWrite error to client.");
					exit1(0,"\nWrite error to client.\n");
				}
			if(-1 == write(clientRDWR,&(q->frequency),sizeof(int)) ){
					//fprintf(stderr,"\nWrite error to client.");
					exit1(0,"\nWrite error to client.\n");
				}
			
		fprintf(stderr,"\n%s:%d\n",q->word,q->frequency);
		}
		q=q->next;
	}
}
//----------------------------------------------------------------------------buckets
WordNode **
 createBucket(WordNode *result, WordNode **bucket){
	bucket = malloc(10 * sizeof(WordNode*));
	if(bucket==NULL){
		//fprintf(stderr,"\nMemory allocation problem.");	
		exit1(0,"\nMemory allocation problem. \n");
	}
	int i=0;
	for(i=0;i<10;i++)
	{
		bucket[i]=NULL;
	} 
	result = malloc(sizeof(WordNode *));
	if(result==NULL){
		//fprintf(stderr,"");	
		exit1(0," \nMemory allocation problem.\n");
	}
	result->frequency=0;
	result->next=NULL;

	return bucket;
}
//-----------------------------------------------------------------------------------Insert in structure STOP WORDS
void insertInStopWord(char *buffer, StopWord *headStopWord)
{
	StopWord *p = headStopWord;
	while(p->next != NULL){
		p=p->next;
	}
	p->next = malloc (sizeof(StopWord));
	p->next->word = malloc(sizeof(strlen(buffer)+1));
	strcpy(p->next->word,buffer);
	p->next->next=NULL;
	p=p->next;
}
//---------------------------------------------------------------------------------- Insert in structure WORDNODES
void insertInStructure(char *wordEntry, WordNode *locator, StopWord *headStopWord, WordNode **bucket)
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
			//if(newMAX<update)
			//	newMAX =update;
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
			//if(newMAX<1)
			//	newMAX=1;
			if(bucket[bucketNumber] == NULL){					//wordEntry not found in the list. & list is empty. 1st entry
				bucket[bucketNumber] =root;
				}else{
				root->next=bucket[bucketNumber];
				bucket[bucketNumber]=root;
				}
		}

}

void display(int argc, char *argv[], WordNode **bucket, WordNode *topfrequentwordlist){

	calculateMAX(bucket, topfrequentwordlist);

	int clientRDWR= openClientFIFO(argv[argc]);
	displayfinalarray(topfrequentwordlist, clientRDWR);
	close(clientRDWR);
	unlink(argv[argc]);
	
}
void calculateMAX(WordNode **bucket, WordNode *topfrequentwordlist){
	int i=0;
	for(i=0;i<10;i++){
		WordNode *p = bucket[i];
		while(p){
			compareWordNode(p, topfrequentwordlist);
			p=p->next;
		}
	}
	
}

void compareWordNode( WordNode *p, WordNode *topfrequentwordlist){
	
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
		deletelast(topfrequentwordlist);
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
			deletelast(topfrequentwordlist);
			break;
			}
		iter=iter->next;
		iter1=iter1->next;
		}
		
	} 
	
		
}
void deletelast(WordNode *topfrequentwordlist){
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


void dofree(WordNode **bucket)
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
// needed
void newmerger(WordNode* list1, WordNode* list2){
	WordNode *p = list1;
		while(p->next){
		p=p->next;
		}
	p->next =list2;
	
}
// needed
int newlength(WordNode *root){
	int length=0;
		WordNode *p = root;
		while(p){
			length++;
			p=p->next;
			}
	
	return length;
		
}
// needed
WordNode* getnode(int max, WordNode **bucket){
	
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
		printf("\n%s %d\n",iter->word, iter->frequency);
		iter=iter->next;
		}
	}
	return q;
}
// needed
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
// needed
int newgetmax(WordNode **bucket){
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
// needed
int newgetmax2(int oldmax, WordNode **bucket){
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
// needed
void newprintfinal(WordNode *root, int N_Words, char *clientpid, int writefd){

	
		int clientRDWR= writefd;
	
		WordNode *p = root;
		while(p && N_Words){
			int wordlength=strlen(p->word)+1;

			
			if(-1 == write(clientRDWR,&wordlength,sizeof(int)) ){
					fprintf(stderr,"\nWrite error to client.\n");
					//exit1(0," ");
				}

			if(-1 == write(clientRDWR,p->word,wordlength) ){
					fprintf(stderr,"\nWrite error to client.\n");
					//exit1(0," ");
				}
			if(-1 == write(clientRDWR,&(p->frequency),sizeof(int)) ){
					fprintf(stderr,"\nWrite error to client.\n");
					//exit1(0," ");
				}
			//fprintf(stdout,"\n%s %d: %s\n",p->word, p->frequency,clientpid);
			p=p->next;
			N_Words--;
			}
	p=root;
	/*while(p){
		WordNode *q= p;
		p=p->next;
		free(q->word);
		//free(q->frequency);
		free(q);
	}*/
	close(clientRDWR);
	unlink(clientpid);
	
}
// needed
void newdisplay(int top_N_Words, char *clientpid, WordNode **bucket, int writefd){

	//printf("\n%d\n",newMAX);
	//printf("\n%d\n",maxfreq);
	int maxfreq;
	maxfreq=newgetmax(bucket);

	WordNode *root = newsort(getnode(maxfreq, bucket));
	while(newlength(root) < top_N_Words && maxfreq > 0){

		maxfreq=newgetmax2(maxfreq,bucket);
		WordNode *temp = getnode(maxfreq, bucket);
		temp = newsort(temp);
		newmerger(root,temp);

	}
	newprintfinal(root, top_N_Words, clientpid, writefd);
	
}

