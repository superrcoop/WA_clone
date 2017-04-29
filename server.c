/* This is a 'Whatsapp' server in the internet domain using TCP
   The port number is passed as an argument. 
   This version runs by forking off a separate 
   process for each connection.
*/

#include <stdio.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h> 
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/wait.h>
#include <time.h>

#define BUF_SIZE 1024
FILE *datastore;
struct sockaddr_in;

/* function prototype */
void error(char);
void create_directories();
void check_user(char);
void user_reg(char);
char getContacts(FILE *fp); 
char welcome();
void sendMessage(int,char,struct sockaddr *s,socklen_t *a);
char receiveMessage(int,struct sockaddr *s,socklen_t *a);
void log_chat(char);

/*---------------------- main ------------------*/
int main(int argc, char *argv[]){
  int sockfd, newsockfd, portno, cli_len, pid,pid2,ret;
  struct sockaddr_in serv_addr,cli_addr,req_addr;
  char clientAddr[cli_len],username[BUF_SIZE],msg[BUF_SIZE];

  /* usage */
  if (argc < 2){
    fprintf(stderr,"ERROR, no port provided\n");
    exit(1);
  }
  
  create_directories(); //install 'whatsapp' database

  /*create socket for receiving */
  sockfd = socket(AF_INET, SOCK_STREAM, 0);
  if (sockfd < 0){ 
    error("ERROR opening socket");
  }
  printf("Socket created.. \n");
    
  /* make local address structure */
  bzero((char *)&serv_addr, sizeof(serv_addr)); //zero out structure
  portno = atoi(argv[1]); //address family
  serv_addr.sin_family = AF_INET; // Current machine IP
  serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
  serv_addr.sin_port = htons(portno);

  /*binding socket*/
  if (bind(sockfd, (struct sockaddr *) &serv_addr,sizeof(serv_addr)) < 0){
    error("ERROR on binding");
  }
  printf("Socket binded.. \n");
  
  
  /*listen for incoming connection*/
  printf("listening for connections.. \n");
  listen(sockfd,5);
  cli_len = sizeof(cli_addr);
  while (1){
    newsockfd = accept(sockfd,(struct sockaddr *) &cli_addr, &cli_len); //create a new socket with new connection
    if (newsockfd < 0){
      error("ERROR on accept");
    }
    pid = fork(); //spawn child process to deal with accepted new connection
    if (pid < 0){
      error("ERROR on fork");
    }

    if (pid == 0){ 
      close(sockfd); //close socket
      inet_ntop(AF_INET, &(cli_addr.sin_addr), clientAddr, cli_len); //parsing
      username=receiveMessage(newsockfd,(struct sockaddr *) &cli_addr,cli_len); //recieves username from client
      check_user(username); //check if user is registered in the database
      printf("Connected to %s %s...\n", clientAddr, username); 
      msg=getContacts(datastore); //loads contact list from file
      sendMessage(newsockfd, msg, (struct sockaddr *) &cli_addr, cli_len); //send contact list to client
      while (1){
        msg=receiveMessage(newsockfd,(struct sockaddr *) &cli_addr,cli_len); //recieve rquested chat user name
        if check_user(msg!=NULL){ //checks to see if requested user is registered
          pid2 = fork();//create child process to prompt requested user to chat with client  
          if (pid2 < 0){
            error("ERROR on fork");
          }
          if (pid2 == 0){
            
            /*req_addr=get_usrIP();         //gets client addr from database file] */           
            msg="chat requested from another user,accept(yes) or decline(no)";
            sendMessage(newsockfd, msg,(struct sockaddr *) &req_addr, cli_len);    //send prompt to requested user. 
            msg=receiveMessage(newsockfd,(struct sockaddr *) &req_addr,cli_len); //recieve response
            
            if (strcmp(msg,'yes')==0){
              chatGranted=1;
              msg="user is online,send a message";
              sendMessage(newsockfd, msg,(struct sockaddr *) &cli_addr, cli_len);
            }
            else{
              chatGranted=0;
            }
          exit(0);
          }//end child process
          /*parent process*/
          pid2=wait((int *)0); //waits on child processto return chat grant
          if (chatGranted==1){
            msg=receiveMessage(newsockfd,(struct sockaddr *) &req_addr,cli_len);
            while ( msg != "quit") {
              sendMessage(newsockfd, msg, BUF_SIZE, 0, (struct sockaddr *) &cli_addr, cli_len);           //sends message to client a
              msg=receiveMessage(newsockfd, msg, BUF_SIZE, 0, (struct sockaddr *) &cli_addr, cli_len);    //recieves response from client a
              log_chat(msg);                                                                              //log chat messages
              sendMessage(newsockfd, msg, BUF_SIZE, 0, (struct sockaddr *) &req_addr, cli_len);           //send message to client b
              msg=receiveMessage(newsockfd, msg, BUF_SIZE, 0, (struct sockaddr *) &req_addr, cli_len);    //recieves response from client b
              log_chat(msg);
            } 
          }else{
            msg="quit";
            sendMessage(newsockfd,msg,(struct sockaddr *)&cli_addr,cli_len); //chat request denied
            exit(0);
          }
        }//end if
      exit(0);
      }//end while
    }//end child
  /*parent process*/  
  printf("Parent process: My child pid is %d \n",pid);
  printf("Parent process: waiting on child");
  pid=wait((int *)0);
  printf("Parent process: Well done my child");
  close(sockfd); //if child process executes this socket would already have been closed
  close(newsockfd); 
  fclose(datastore);
  printf("Connection socket disconnected (%s) \n", clientAddr); 
  exit(0);
}
  return 0;
}//end main


/*------------------------------------- helper function ---------------------------------*/

char getContacts(FILE *database)
{
  
  int n;
  char buffer[BUF_SIZE];  

  buffer="Getting Contacts from file..";

  /*function read from the database and displays online contacts [menu prompt: GROUPS,CONTACTS or EXIT] from database */
  return buffer; 
}

char receiveMessage(int socket,struct sockaddr *src_addr, socklen_t *addrlen) {
  int ret;
  char buffer[BUF_SIZE]; 
  memset(buffer, 0, BUF_SIZE);  


  ret = recvfrom(socket, buffer, BUF_SIZE, 0, src_addr, addrlen);  
  if (ret < 0) {  
    printf("Error receiving data!\n");    
  }else{
    return buffer;
  }  
}

void sendMessage(int socket,char msg,struct sockaddr *rcv_addr,socklen_t *addrlen) {
  /*send message to another client, Parameters should be (<socket>,<sender>,msg,<recieverAddr>)*/
  int ret;
  ret = sendto(socket, msg, BUF_SIZE, 0, rcv_addr, addrlen);  
  if (ret < 0) {  
    printf("Error receiving data!\n");    
  }  
}

void check_user(char *username) /*this function checks if user is already registered*/
{
  char command[100] = "grep ";
  strcat(strcat(command,username)," /tmp/Whatsapp/master_file/data.txt > /dev/null");

  if(system(command)!= 0){
    user_reg(username);

  }
  else{
    puts(welcome());
  }
}

void error(char msg){
  puts(msg);
  exit(1);
}

void user_reg(char *usr_data)
{
    //open file
    fprintf(datastore,"%s\n",usr_data);
    fclose(datastore);
}

void log_chat(char message)
{
  /*logs chat messages in database*/
    time_t mytime;
    struct tm *info;
    mytime = time(NULL);
    char str[80];
    char input[80];
    time( &mytime );
    info = localtime( &mytime );
    strftime(str,80,"%x -%I:%M%p --> ",info);
    datastore = fopen("/tmp/Whatsapp/chat_logs/log.txt","a");//creates registration log file
    fprintf(datastore,"%s", message);         
    fclose(datastore);
}

void create_directories()
{
    if (system("ls /tmp/Whatsapp > /dev/null")!= 0)//checks if directory already exist
    {
        system("mkdir -p /tmp/Whatsapp/master_file"); //creates directory to store data
        system("mkdir /tmp/Whatsapp/chat_logs");//creates directory for chat logs
    }
        
    datastore = fopen("/tmp/Whatsapp/master_file/data.txt","a");//creates registration log file
}
