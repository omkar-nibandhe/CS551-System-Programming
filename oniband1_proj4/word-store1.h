#include<stdlib.h>
#include<stdio.h>
#include<string.h>
#include <ctype.h>



typedef struct WordNode {
	char *word;
	int frequency;
	struct WordNode* next;
}WordNode;
typedef struct StopWord {
	char *word;
	struct StopWord* next;
}StopWord;



void createStopWordList();
WordNode* search(char*);
WordNode* createFinalArray( int top_N);
void displayfinalarray(WordNode * topfrequentwordlist, int clientRDWR);
void createBucket();
void insertInStopWord(char *buffer);
void insertInStructure(char *wordEntry);
void display(int argc, char *argv[]);
void calculateMAX();
void compareWordNode( WordNode *p);
void deletelast();
void dofree();
void newmerger(WordNode* list1, WordNode* list2);
int newlength(WordNode *root);
WordNode* getnode(int max);
WordNode* newsort(WordNode* something);
int newgetmax();
int newgetmax2(int oldmax);
void newprintfinal(WordNode *root, int N_Words, char *clientpid);
void newdisplay(int top_N_Words, char *clientpid);
