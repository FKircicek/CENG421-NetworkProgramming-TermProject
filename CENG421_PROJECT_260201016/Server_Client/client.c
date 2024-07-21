// To compile: gcc client.c  -o client
// To run ./client <IP> <port> <username>
// ctrl + c or type 'exit' to leave the chat

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <pthread.h>
  
#define MSG_LEN     1024
#define NAME_LEN    32
  

void* send_message(void *arg);
void* receive_messageg(void *arg);
void error_handler(char *err_msg);
  
// global variables 
char name[NAME_LEN] = "noname";
char message[MSG_LEN];
char info_msg[MSG_LEN];
  
  
int main(int argc, char *argv[])
{
    //variables
    int sock;                           
    struct sockaddr_in sv_addr;         
    pthread_t send_thread, receive_thread; 
    void *thread_return;                
  
    // handle invalid number of arguments
    if (argc != 4)
    {
        printf("Usage: %s <IP> <port> <username>\n", argv[0]);
        exit(1);
    }
  
    sprintf(name, "%s", argv[3]);  // setup username
  
    // create a TCP socket
    sock = socket(PF_INET, SOCK_STREAM, 0); 
  
    //server settings
    memset(&sv_addr, 0, sizeof(sv_addr));
    sv_addr.sin_family = AF_INET;
    sv_addr.sin_addr.s_addr = inet_addr(argv[1]);
    sv_addr.sin_port = htons(atoi(argv[2]));
  
    // server connection
    if (connect(sock, (struct sockaddr*)&sv_addr, sizeof(sv_addr)) == -1)
        error_handler("connect() error");

    printf("Group chat application (type 'exit' to quit)\n\n");
  
    sprintf(message, "#### %s has joined ####\n", name);
    write(sock, message, strlen(message));
  
    // create threads
    pthread_create(&send_thread, NULL, send_message, (void*)&sock);
    pthread_create(&receive_thread, NULL, receive_messageg, (void*)&sock);
    pthread_join(send_thread, &thread_return);
    pthread_join(receive_thread, &thread_return);
  
    // terminate connection
    close(sock);
  
    return 0;
}
  
// send message to the server
void* send_message(void *arg)
{
    int sock = *((int*)arg);
    char name_msg[NAME_LEN + MSG_LEN + 2];
  
    while (1)
    {
        fgets(message, MSG_LEN, stdin);
  
        // terminate the connection upon user input 'exit'
        if (!strcmp(message, "exit\n"))
        {
            
            sprintf(message, "#### %s has left the chat ####\n", name);
            write(sock, message, strlen(message));
  
            close(sock);
            exit(0);
        }
  
        // construct the message and send it to the server
        sprintf(name_msg, "%s: %s", name, message);
        write(sock, name_msg, strlen(name_msg));
    }
      
    return NULL;
}
  
// receive message and print it to the screen
void* receive_messageg(void *arg)
{
    int sock = *((int*)arg);
    char name_msg[NAME_LEN + MSG_LEN + 2];
    int str_len;
  
    while(1)
    {
        str_len = read(sock, name_msg, NAME_LEN + MSG_LEN + 2 - 1); 
  
        // terminate the thread
        if (str_len == -1)
            return (void*)-1;
  
        name_msg[str_len] = 0;
        fputs(name_msg, stdout);
    }
  
    return NULL;
}
  
// error handler
void error_handler(char *err_msg)
{
    fputs(err_msg, stderr);
    fputc('\n', stderr);
    exit(1);
}