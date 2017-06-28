#include"word-store1.h"

//---------------------------------------------------------------------------------STOP WORD
void createStopWordList(){
	headStopWord = malloc(sizeof(StopWord));
	if(headStopWord == NULL){
			fprintf(stderr,"\nMem alloc problem\n");			
			exit(0);
			}
	headStopWord->next = NULL;

	topfrequentwordlist = malloc (sizeof(WordNode));
	if(topfrequentwordlist == NULL){
			fprintf(stderr,"\nMem alloc problem\n");			
			exit(0);
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
			fprintf(stderr,"\nMem alloc problem\n");			
			exit(0);
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
					fprintf(stderr,"\nWrite error to client.");
					exit(0);
				}

			if(-1 == write(clientRDWR,q->word,wordlength) ){
					fprintf(stderr,"\nWrite error to client.");
					exit(0);
				}
			if(-1 == write(clientRDWR,&(q->frequency),sizeof(int)) ){
					fprintf(stderr,"\nWrite error to client.");
					exit(0);
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
		fprintf(stderr,"\nMemory allocation problem.");	
		exit(0);
	}
	int i=0;
	for(i=0;i<10;i++)
	{
		bucket[i]=NULL;
	} 
	result = malloc(sizeof(WordNode *));
	if(result==NULL){
		fprintf(stderr,"\nMemory allocation problem.");	
		exit(0);
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
			fprintf(stderr,"\nError creating new node in bucket!\n");
			exit(0);
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

	int clientRDWR= openClientFIFO(argv[argc]);
	displayfinalarray(topfrequentwordlist, clientRDWR);
	close(clientRDWR);
	unlink(argv[argc]);
	
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
			fprintf(stderr,"\n Mem alloc~\n");
			exit(0);
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
				fprintf(stderr,"\n Mem alloc~\n");
				exit(0);
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
						fprintf(stderr,"\nError in malloc\n");
						exit(1);
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
		int clientRDWR= openClientFIFO(clientpid);
		WordNode *p = root;
		while(p && N_Words){
			int wordlength=strlen(p->word)+1;
			if(-1 == write(clientRDWR,&wordlength,sizeof(int)) ){
					fprintf(stderr,"\nWrite error to client.");
					exit(0);
				}

			if(-1 == write(clientRDWR,p->word,wordlength) ){
					fprintf(stderr,"\nWrite error to client.");
					exit(0);
				}
			if(-1 == write(clientRDWR,&(p->frequency),sizeof(int)) ){
					fprintf(stderr,"\nWrite error to client.");
					exit(0);
				}	
			//fprintf(stdout,"\n%s %d: %s\n",p->word, p->frequency,clientpid);
	
			N_Words--;	
			p=p->next;
			}
	close(clientRDWR);
	unlink(clientpid);
	
}
void newdisplay(int top_N_Words, char *clientpid){

	//printf("\n%d\n",newMAX);
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

