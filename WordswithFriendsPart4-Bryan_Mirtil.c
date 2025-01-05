#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>    
#include <netinet/in.h>  
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include <netdb.h> 
#include <pthread.h>
#include <stdbool.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <ctype.h>
#include <errno.h>

void gameLoop();
int initialization();
void gameLoop();
void tearDown();
void getLetterDistribution();
bool compareCounts();
bool isDone();
char* getRandomWord();
void acceptInput(char *input, void *new_fd);
void displayWorld();
struct gameListNode* findWords(char*, int*);
bool compareCounts2(int*, int*);
void cleanupGamelistNodes(struct gameListNode* top);
void cleanupGamelistNodes(struct gameListNode* top);
void print(struct gameListNode* top);
void *newMain(void *new_fd);



int Letter[26];
int compares[26];
int Find[26];
char Word[100];
char *MasterWord = "";

struct gameListNode* top = NULL;
struct gameListNode* follow = NULL;

struct WordListNode* head = NULL;
struct WordListNode* current = NULL;
int count = 0;


struct gameListNode{
char gameList[30];
bool found;
struct gameListNode* next;
};

struct WordListNode {
char WordList[30];
struct WordListNode *next;
};

struct myParams {pthread_t id;};

void *Receive(void *value);
char command[300]; 

//makes the socket for the user to connect to from the browser and gets the command line arguments
//and stores them in character arrays and makes threads for each run
int main(int argc, char **argv){
strcpy(command,argv[1]);

int len = 100;
char buffer[50];
int port = 8000;
sprintf(buffer,"%d", port);
int sockfd, new_fd, yes = 1, rv;
struct addrinfo hints, *servinfo, *p;
struct sockaddr_storage their_addr;
socklen_t sin_size;	
memset(&hints, 0, sizeof(hints));
hints.ai_family = AF_INET;
hints.ai_socktype = SOCK_STREAM;
hints.ai_flags = AI_PASSIVE;
rv = getaddrinfo(NULL, buffer , &hints,&servinfo);


for(p = servinfo; p != NULL; p = p->ai_next){
	if((sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1) continue;
	if(bind(sockfd, p->ai_addr, p->ai_addrlen) == -1){
		close(sockfd);continue;
		send(sockfd, "error getting info", len, 0);
	}

	freeaddrinfo(servinfo);
	listen(sockfd, 1);
	
	while(1){
		int *new_fd = malloc(sizeof(int));
		sin_size = sizeof(their_addr);
	
	*new_fd = accept(sockfd, (struct sockaddr *)&their_addr, &sin_size);
	
	if(*new_fd == -1){
		perror("accept failed");
		free(new_fd);
		continue;
	}
	
	pthread_t thread;
		pthread_create(&thread, NULL, newMain,(void *)new_fd);
		pthread_detach(thread);
		
	}

}
return 0;
}

// thread method that handles the request the user wants 
void *newMain(void *new_fd){
	char buff[8000];
	int len = 500;
	char Words[200];
	char message[100];
	char* token;
	char* path;
	char* fpath;
	char* save1;
	char* save2;
	memset(buff, '\0', strlen(buff));
	
	struct stat* stats = (struct stat*)malloc(sizeof(struct stat));	
	struct myParams *myParams = ((struct myParams *)new_fd); 
	
	
		recv(*((int *) new_fd), buff, len, 0);
	
	printf("request: %s\n", buff);
	// if the URL just contains /words and not ?move=
	if(strstr(buff,"words") != NULL && strstr(buff,"?move=") == NULL){
	initialization();
	printf("masterword: %s\n", MasterWord);
	
	struct WordListNode* tem = head;
	
	while(tem != NULL){
		printf("Wordlist: %s\n",tem->WordList);
		tem = tem->next; 
	}
	findWords(MasterWord, Letter);
	struct gameListNode* temp = top;
	while(temp != NULL){
		printf("Gamelist: %s\n",temp->gameList);
		temp = temp->next; 
	}
	displayWorld(new_fd);
	
	} else if(strstr(buff, "?move=") != NULL) { // if the URL contains ?move= then we'll handle words sent
		if(!isDone()){
	acceptInput(buff, new_fd);

	memset(buff, '\0', strlen(buff));
	sleep(2);
	//int r = recv(*((int *) new_fd), buff, sizeof(buff), 0);
	fflush(stdout);
	
	//if(r == -1){
	//	printf("%s\n",strerror(errno));
	//}
	
	}
	
	if (isDone()){
	printf("hello Done\n");
	//tearDown(new_fd);
	//close(*((int *)new_fd));
	
	char finish[100];
	char headlen[300];

	char *msg = "<html><body>Congratulations! You solved it! <a href=\"words\">Another?</a></body></html>";

	strcpy(finish, msg);
	int hlength = strlen(finish);
	snprintf(headlen,sizeof(headlen), ("HTTP/1.1 200 OK\r\nContent-Length:%d\r\n\r\n"),hlength);

	char finalbuff[500];
	snprintf(finalbuff, sizeof(headlen),"%s",headlen);
	strcat(finalbuff, finish);

	printf("sending: %s\n", finalbuff);
	tearDown(new_fd);
	send(*(int *)new_fd, finalbuff, strlen(finalbuff), 0);
	
	close(*((int *)new_fd));
	return NULL;
	}
	}else{
	
	// if the URL doesn't contain words or ?move= 
	//parses the GET and the path and concatenates it to find and open the file needed then opens it and 
	//copies all the text into a buffer and then sends it to the broswer to print and if not then it prints a 404 error
	token = strtok_r(buff, " ", &save1);
	path = strtok_r(save1, " ", &save2);
	fpath = &path[1];
	
	if(strcmp(token, "GET") == 0){
	strcat(command, fpath);
	int file = open(command, 0, O_RDONLY);
	stat(fpath, stats); 	
		
	if(file != -1){
	read(file, Words,stats->st_size);
	int l = snprintf(message,sizeof(message),"HTTP/1.1 200 OK\r\nContent-Length:%ld\r\n\r\n", stats->st_size);
	send(*(int* )new_fd,message,strlen(message),0);
	send(*(int* )new_fd,Words,stats->st_size,0);
	close(*((int *)new_fd));
	close(file);
	free(stats);
	return NULL;
		}
	char fail[100];
	char* body = "<p>Word not found</p>";
	int bodylength = strlen(body);
	snprintf(fail,sizeof(fail), ("HTTP/1.1 200 OK\r\nContent-Length: %d\r\n\r\n%s"),bodylength, body);
	displayWorld(new_fd);
	send(*(int *)new_fd, fail, strlen(fail), 0);
	close(*((int *)new_fd));
	close(file);
	free(stats);
	return NULL;
	}
	
}
}

//sends html of the masterword and every word to be found and displays if you found it or not for each input
void displayWorld(void *new_fd){
	
	int len = 4000; 
    char *sendbuff = (char *)malloc(len * sizeof(char));
	char headlen[100];


const char *sender = "<html><body>";
strcat(sendbuff,sender);


struct gameListNode* temp = top;
int i = 0;
int j = 0;

int length = strlen(MasterWord);


for(int i = 0; i < length -2 ;i++){
	for(int j = 0; j < length - i - 2;j++){
		if(MasterWord[j] % 65 > MasterWord[j+1] % 65){
			char copy = MasterWord[j];
			MasterWord[j] = MasterWord[j+1];
			MasterWord[j+1] = copy;
		}
	}
	
}
printf("sorted masterword: %s\n",MasterWord);

strcat(sendbuff,MasterWord);


strcat(sendbuff,"<p></p>");
strcat(sendbuff,"<p>-----------------------</p>");


while(temp != NULL){
	strcat(sendbuff, "<p>");
for(int i = 0; i < strlen(temp->gameList)-1;i++){
strcat(sendbuff,"_ ");

}
if(temp->found == true){
	char messagefound[50];
		snprintf(messagefound,sizeof(messagefound), "Found : %s\n",temp->gameList);
		strcat(sendbuff, messagefound);
	}

strcat(sendbuff, "</p>");

temp = temp->next;	
}

if(isDone() == true){
	strcat(sendbuff, "<p>");
	strcat(sendbuff, "You're all Done");
	strcat(sendbuff, "<p>");
	//tearDown(new_fd);
}


const char *enter = "<form submit=\"words\">Enter a word: <input type=\"text\" name=move autofocus>";
	strcat(sendbuff,enter);



const char *ender = "</input></form></body></html>";
	strcat(sendbuff,ender);
	
int hlength = strlen(sendbuff);
snprintf(headlen,sizeof(headlen), ("HTTP/1.1 200 OK\r\nContent-Length:%d\r\n\r\n"),hlength);




char finalbuff[8000];
snprintf(finalbuff, sizeof(headlen),"%s",headlen);
strcat(finalbuff, sendbuff);


if(send(*(int *)new_fd, finalbuff, strlen(finalbuff), 0) == -1){
	perror("error sending buffer");
}
free(sendbuff);
memset(headlen, '\0', sizeof(headlen));
memset(finalbuff, '\0', sizeof(finalbuff));
memset(sendbuff, '\0', sizeof(sendbuff));

}

//takes in the user input and checks to see if the user word is within the game list
void acceptInput(char *buff, void *new_fd){
char input[30];	
char* keep = buff; 
char* save1;
char* save2;
char* token;
char* path;

printf("accepting input\n");

token = strtok_r(keep, "=", &save1);
path = strtok_r(save1, " ", &save2);

if(path == NULL){
		return;
}

strcpy(input, path);
printf("input: %s\n", input);
struct gameListNode* temp = top;




for(int i = 0; input[i] != '\0'; i++){
	
	input[i] = toupper(input[i]);
	if(input[i] == '\n' || input[i] == '\r'){
		input[i] = '\0';
	}
}


while(temp != NULL){
	printf("temp: %s\n", temp->gameList);
if(strcmp(temp->gameList, input) == 0){
printf("%d\n",strcmp(temp->gameList, input));
temp->found = true;
 printf("Matched! Setting temp->found for: %s\n", temp->gameList);
memset(input, '\0', sizeof(input));
if(isDone() == true){
		return;
	}
if (!isDone()) {
	displayWorld(new_fd);
//return;
//if(temp->next == NULL){
	//if (isDone())) {
	//displayWorld(new_fd);
	//printf("finished");
	return;
	}
	
//}	
// temp = temp->next;
}
temp = temp->next;
}
displayWorld(new_fd);
memset(input, '\0', sizeof(input));
return;
//if(temp == NULL){
	//char fail[100];
	//char* body = "<p>Word not found</p>";
	//int bodylength = strlen(body);
	//snprintf(fail,sizeof(fail), ("HTTP/1.1 200 OK\r\nContent-Length: %d\r\n\r\n%s"),bodylength, body);
	//displayWorld(new_fd);
	//send(*(int *)new_fd, fail, strlen(fail), 0);
	//memset(input, '\0', sizeof(input));
	//printf("hello8\n");
	//return;
//}
//return;
}



//creates the gamelist linked list from the wordlist linked list of words that
//can be made from the masterword 
struct gameListNode* findWords(char *word, int *letter){
	
struct WordListNode* temp = head;
getLetterDistribution(MasterWord, Letter);

printf("\n");


for(int i = 0; i < count; i++){
getLetterDistribution(temp->WordList,compares);

if(compareCounts2(compares, Letter) == false){
	temp = temp->next;
}
if(compareCounts2(compares, Letter) == true){
struct gameListNode* Node = (struct gameListNode*)malloc(sizeof(struct gameListNode));

strcpy(Node->gameList, temp->WordList);
Node->found = false; 
Node->next = NULL;

if(top == NULL){
	top = Node;
	follow = top;
	temp=temp->next;
}else{
follow->next = Node;
follow = follow->next; 
temp=temp->next;

}

}
for(int i = 0; i < 26; i++){
	compares[i] = 0;
}

}

return top;
}


//compares the two integer arrays and checks if the current word in the linked list
//linked list can be made from the masterword
bool compareCounts2(int *word, int *letter){
	
	for(int i = 0; i < 26; i++){
		if(compares[i] > Letter[i]){
			
			return false;
		}
		
	}
	return true;
}



//creates a linked list from the text file and makes each word uppercase
int initialization(){
FILE* fp;
fp = fopen("/mnt/c/Users/bryan/OneDrive/Documents/2of12.txt", "r");
char setbuffer[30];

while(fgets(setbuffer, sizeof(setbuffer), fp) != NULL){
struct WordListNode* Node = (struct WordListNode*)malloc(sizeof(struct WordListNode));

for(int i = 0; setbuffer[i]; i++){
setbuffer[i] = toupper(setbuffer[i]); 
if(setbuffer[i] == '\n' || setbuffer[i] == '\r'){
		setbuffer[i] = '\0';
	}
}
strcpy(Node->WordList, setbuffer);
Node->next = NULL;	
count++;

if(head == NULL){
	head = Node;
	current = head;
}else{
current->next = Node;
current = Node; 
}

}
fclose(fp);
MasterWord = getRandomWord();

return count;
}




//searches the linked list and returns a random node bigger than 6
char* getRandomWord(){

srand(time(0));
int random = rand() % count;

struct WordListNode* temp = head;	

for(int i = 0; i <= random;i++){
temp = temp->next;
}

if(strlen(temp->WordList) > 4){

return temp->WordList;
}

while(strlen(temp->WordList) <= 4){
temp = temp->next;
if(temp == NULL){

return getRandomWord();
}
}

printf("%s\n ",temp->WordList);
return temp->WordList;
}






// finds the position of the letter in the array by using the modulo
// division to give us remainder which will tell the position the letter is in the array
void getLetterDistribution(char *word, int *letter){
	int Number = 0;
	for(int i = 0; word[i]; i++){
		if(isalpha(word[i]))
		{
			Number = word[i] % 'A';
			letter[Number]++;
		}
		
	}
}


// compares the user input word to the masterword given to us and  
// returns true if the count of letters is less than or equal to the masterword
// and false otherwise
bool compareCounts(){
	
	getLetterDistribution(Word, Find);
	for(int i = 0; i < 26; i++){
		if(Find[i] > Letter[i]){
			return false;
		}
	}
	return true;
}

//checks to see if the user found all the words in the game linked list 
bool isDone(){
struct gameListNode* temp = top;
while(temp != NULL){
if(!temp->found){
printf("current word: %s\n", temp->gameList); 
return false;	
}
//printf("current word: %s\n", temp->gameList); 
//printf("current word found: %d\n", temp->found); 
temp = temp->next;
}
//return false;

return true;
}

//frees all the memory used up in the program
void cleanupGamelistNodes(struct gameListNode* top){
struct gameListNode* temp = top;

while(top != NULL){
temp = top;
top = top->next;
free(temp);
}
}

//frees all the memory used up in the program
void cleanupWordlistNodes(struct WordListNode* head){
struct WordListNode* temp;

while(head != NULL){
temp = head;
head = head->next;
free(temp);
}
}
//ends the program and frees all the allocated memory
void tearDown(void *new_fd){
printf("reached tearDown\n");
cleanupWordlistNodes(head);
cleanupGamelistNodes(top);
/*
char finish[100];
char headlen[300];

char *msg = "<html><body>Congratulations! You solved it! <a href=\"words\">Another?</a></body></html>";

strcpy(finish, msg);
int hlength = strlen(finish);
snprintf(headlen,sizeof(headlen), ("HTTP/1.1 200 OK\r\nContent-Length:%d\r\n\r\n"),hlength);

char finalbuff[500];
snprintf(finalbuff, sizeof(headlen),"%s",headlen);
strcat(finalbuff, finish);

printf("sending: %s\n", finalbuff);

send(*(int *)new_fd, finalbuff, strlen(finalbuff), 0);
*/
}
	