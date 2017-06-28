//#include"word-store.h"
//void display();
//void calculateMAX();
//void deletelast();
//void compareWordNode( WordNode *p);
//void dofree();

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

void displayfinalarray(WordNode * topfrequentwordlist){
	WordNode* q = topfrequentwordlist;
	//printf("\n------Display Final-------");
	while(q != NULL){
		if(q->word != NULL)
			fprintf(stdout,"\n%s %d\n",q->word, q->frequency);
		q=q->next;
	}
	//printf("\n---------------------------\n");
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
		//bucket[i] = malloc(sizeof(WordNode));
		//bucket[i]->frequency = 0;
		//bucket[i]->next = NULL;
		bucket[i]=NULL;
		//strcpy(bucket[i]->word,"");
	} 
	result = malloc(sizeof(WordNode *));
	if(result==NULL){
		fprintf(stderr,"\nMemory allocation problem.");	
		exit(0);
	}
	result->frequency=0;
	result->next=NULL;
	//getchar();
}
//-----------------------------------------------------------------------------------Insert in structure STOP WORDS
void insertInStopWord(char *buffer)
{
	//printf("\n%zd:%s",strlen(buffer),buffer);

	p->next = malloc (sizeof(StopWord));
	p->next->word = malloc(sizeof(strlen(buffer)+1));
	strcpy(p->next->word,buffer);
	p->next->next=NULL;
	p=p->next;
}
//---------------------------------------------------------------------------------- Insert in structure WORDNODES
void insertInStructure(char *wordEntry, int freq)
{
	WordNode *root;						//code to isnert word on hashin function.
	//printf("\n-%s\n",wordEntry);
	StopWord *q = headStopWord;
	q=q->next;
	while(q){
		if(strcmp(q->word,wordEntry)==0)
			return;
		q=q->next;
	}
	int bucketNumber= (strlen(wordEntry)%10);
	
	locator = bucket[bucketNumber];
	//getchar();
	while(locator != NULL) {	
		//getchar();
		//1111111111locator=locator->next;							//search word
		if(strcmp(locator->word, wordEntry) == 0){	//word already exits so increment frequency and exit insertfunciton
			int update = locator->frequency;
			update= update+freq;													//mark change
			if(newMAX<update)
				newMAX =update;
			locator->frequency = update;
			//printf("\n %s Already exists! now freq is %d",wordEntry,locator->frequency);
			return ;
			}
		locator=locator->next;	
		}
	//display();	

						//word does not exists.
	locator = bucket[bucketNumber];					//wordEntry not found in the list.
		
		root = malloc(sizeof(WordNode));
		if(root == NULL){
			fprintf(stderr,"\nError creating new node in bucket!\n");
			//getchar();
			exit(0);
		}else{
			root->word= malloc(sizeof(strlen(wordEntry)+1));
			root->frequency=freq;
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
			/*	strcpy(root->word,wordEntry);				//copy wordEntry
				root->frequency=1;						//set frequency
				root->next=NULL;						//set next pointer to null
			}else{
									//wordEntry not found in the list. & list exits. Traverse till end and insert.

				while(locator->next != NULL){
					locator= locator->next;	
				}

				locator->next = root; 
				strcpy(locator->word,wordEntry);				//copy wordEntry
				locator->frequency=1;						//set frequency
				locator->next=NULL;						//set next pointer to null
				
	
			}*/
		}

}
void mylistinsert(char *buffer, WordNode *head){
	
	//printf("\n-%zd \n",strlen(buffer));
	WordNode *itra= head;
			
			while(itra->next != NULL){
				itra=itra->next;	
				if(strcmp(itra->word,buffer)==0){
					int update =itra->frequency;
					update =update+1;
					itra->frequency=update;
					return;
				}
			}
	if(strlen(buffer)>0){
		WordNode *newnode = malloc(sizeof(WordNode)*sizeof(char));
		newnode->word= malloc(sizeof(char)*(sizeof(strlen(buffer))+1) );
		if(newnode == NULL || newnode->word == NULL){
			fprintf(stderr,"\n Mem alloc~\n");
			exit(-1);
			}
		newnode->next=NULL;
		newnode->frequency=1;
		strcpy(newnode->word,buffer);


		if(NULL == head->next){
			head->next=newnode;
			
			//head->next->next=NULL;
		}else{
			
				itra = head->next;
				
				newnode->next = itra;
				
				head->next = newnode;
				}

			}

		}



void displayChildList(WordNode *head){
	WordNode *p=head;
	while(p !=NULL){
		p=p->next;
		printf("\n %s %d",p->word,p->frequency);
	}
}
void display(){
	int i=0;
	if(i){	
	for(i=0;i<10;i++)
	{
		WordNode  *iterat  = bucket[i];
		while(iterat){
		//if(iterat->word!=NULL)
		printf("\n\t%s %d",iterat->word, iterat->frequency);
		iterat=iterat->next;
		}
	}
	}else{
	
	//printf("\nFinal results:\n");
	//displayfinalarray(topfrequentwordlist);
	calculateMAX();
	displayfinalarray(topfrequentwordlist);
	printf("\n");
	}

	
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
	//printf("\n\tcomparewordnode~ %s : %d",p->word, p->frequency);
		//displayfinalarray(topfrequentwordlist);
	
	WordNode *iter = topfrequentwordlist;
	if(iter->frequency <= p->frequency){			//1st element to be replaced. and last in linked list to be deleted.
		WordNode *newnode = malloc(sizeof(WordNode));
		if(newnode == NULL){
			fprintf(stderr,"\n Mem alloc~\n");
			exit(0);
			}
		newnode->word = p->word;
	//	printf("\nhead~~~~~%s %s\n",newnode->word, p->word);
		newnode->next = iter;
		newnode->frequency = p->frequency;
		topfrequentwordlist=newnode;
		deletelast();
	}else{
	iter=iter->next;
	WordNode *iter1 = topfrequentwordlist;
	//printf("\n\telse~ %s : %s",iter->word, iter1->word);
	//getchar();
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

int counterlist(WordNode *head){
	WordNode *p=head->next;
	int count=0;
	while(p){
		count++;
		p=p->next;
	}
	return count;

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
		//printf("\n%s %s",p->word,q->word);
	}
	}
	/*
	if(counter<=0)	
	{
		printf("\nIndex out of bound! Unable to delete at index: %d\n",index);
		free(p->word);
		free(p);
		return (-1);
	}
	else{
	counter=0;
	p=bucket[3];
	while((counter<(index-1))&&(p!=NULL))
	{
		p=p->next;
		counter=counter+1;
	}
	int deletedvalue=p->next->data_ptr->val;
	free(p->next->data_ptr);
	free(p->next);
	p->next=p->next->next;
	
	return(deletedvalue);
	}
	}*/



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
int newlength(WordNode *root){
	int length=0;
		WordNode *p = root;
		while(p){
			length++;
			p=p->next;
			}
	
	return length;
		
}
void newprintfinal(WordNode *root, int N_Words){

	//int length=0;
		WordNode *p = root;
		while(p && N_Words){
				
			fprintf(stdout,"\n%s %d\n",p->word, p->frequency);
			N_Words--;	
			p=p->next;
			}
	
}
void newmerger(WordNode* list1, WordNode* list2){
	WordNode *p = list1;
		while(p->next){
		p=p->next;
		}
	p->next =list2;
	
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
void newswap(char * smaller, char * longer){
	/*	
	char * temp =malloc(sizeof(longer));
	strcpy(temp,longer);

	int shortlength=strlen(smaller)+1;
	longer=realloc(longer,shortlength);
	strcpy(longer,smaller);

	int longlength=strlen(longer)+1;
	smaller=realloc(smaller,longlength);
	strcpy(smaller,temp);
	*/
	//WordNode *temp=longer;

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
void newdisplay(int top_N_Words){

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
	newprintfinal(root, top_N_Words);
	
}


