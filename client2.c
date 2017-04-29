/* This is a 'Whatsapp' client in the internet domain using TCP,
   The port number and username is passed as an argument.

    620085730 and 05026664 */

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

#define BUF_SIZE 1024
#define SERVER_IP "127.0.0.1"
FILE *datastore, *group, *work;

/* function prototype */
void error(char);
char user_reg(char,struct sockaddr *n);
void sendMessage(int,char,struct sockaddr *s,int);
char receiveMessage(int,struct sockaddr *n,int);

/*---------------------- main ------------------*/
int main(int argc, char *argv[])
{
    int sockfd, portno, n;
    struct sockaddr_in serv_addr;
    struct hostent *server;
    char buffer[BUF_SIZE],user_data[100],serveraddr;

    /*usage*/
    if (argc < 3) {
       fprintf(stderr,"usage %s : client <username> <portno>\n", argv[0]);
       exit(0);
    }
    /*create socket */
    sockfd = socket(AF_INET, SOCK_STREAM, 0); 
    if (sockfd < 0) {
        error("ERROR opening socket");
    }
 
    /* make local address structure */
    portno = atoi(argv[3]); //converts string to integer
    bzero((char *)&serv_addr, sizeof(serv_addr)); //zero out structure
    serv_addr.sin_family = AF_INET; // Current machine IP
    serv_addr.sin_addr.s_addr = htonl(SERVER_IP);
    serv_addr.sin_port = htons(portno);

    /* connecting to socket*/
    if (connect(sockfd,(struct sockaddr *)&serv_addr,sizeof(serv_addr)) < 0){
        error("ERROR connecting");
    }
    inet_ntop(AF_INET, &(serv_addr.sin_addr), serveraddr, n);
    user_data=user_reg(argv[1],serveraddr);//stores client username and address
    sendMessage(sockfd,user_data,(struct sockaddr *)&serv_addr,sizeof(serv_addr));//send user data to server for registration
    
    /*accept client option(exit) or requested chat user name*/
    welcome(); 
    printf(">> ");
    bzero(buffer,BUF_SIZE);
    fgets(buffer,BUF_SIZE,stdin);
    sendMessage(sockfd,buffer,(struct sockaddr *)&serv_addr,sizeof(serv_addr)); //send username to server
    while(1){ 
        puts(receiveMessage(sockfd,(struct sockaddr *)&serv_addr,sizeof(serv_addr)));
        puts("send message: ");
        fgets(buffer, 1000, stdin);
        if (strcmp(buffer,"quit\n") == 0)
            break;
        sendMessage(sockfd,buffer,(struct sockaddr *)&serv_addr,sizeof(serv_addr));
        puts("message sent");
    }//end while
    close(sockfd);//close socket
    printf("Connection ended\n");
}//end main


/*------------------------------------- helper function ---------------------------------*/
void error(char *msg)
{
    perror(msg);
    exit(0);
}

char receiveMessage(int socket,struct sockaddr *src_addr, socklen_t *addrlen) { 
  /*recieve message from another client, Parameters should be (<socket>,<senderAddr>,<len>)*/
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

void sendMessage(int socket,char msg,struct sockaddr *snd_addr, socklen_t *addrlen) {
  /*send message to another client, Parameters should be (<socket>,msg,<recieverAddr>,<len>)*/
  int ret;
  ret = sendto(socket, msg, BUF_SIZE, 0, snd_addr, addrlen);  
  if (ret < 0) {  
    printf("Error receiving data!\n");    
  }  
}

char user_reg(char username,struct sockaddr *sock_addr)
{
    char user_data[100], choice[1];

    //Request inout from user for registration
    puts("You need to register to use this service...");
    puts("Do you wish to continue? [y] for yes and [n] for no");
    scanf("%s", choice);
    strcmp(choice,"y")?exit(0):0;

    //Concatenates user input and write to file
    strcat(user_data,strcat(username,sock_addr));
    return user_data;
}


void welcome()
{
    char greeting[] = "Welcome...The following are potentially available for chat\n"+
                      "______________________________________________________\n"+
                        "Enter:\n"+
                        "1. Username to find friends and start a chat enter \n"+
                        "2. Work Group enter groupchat\n"+
                        "3. Fun Group enter groupchat\n"+
                        
                        "To exit enter [0]\n";

    puts(greeting);    
}

