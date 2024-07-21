//To compile: gcc server.c -o server
//To run: ./server <port>
//Ctrl + c to exit server
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <pthread.h>  


  
#define MSG_LEN     2048
#define MAX_CLIENT  10

void* receive_message(void *arg);
void send_message(char *msg, int len);
void error_handler(char *err_msg);
  
// global variables 
int number_of_clients = 0;
int list_of_clients[MAX_CLIENT];
pthread_mutex_t mutex;  
  
  
int main(int argc, char *argv[])
{
    //variables for socket
    int server_socket, client_socket;
    struct sockaddr_in server_address, client_address;
    int client_address_size;
    pthread_t thread_id;
  
    // handle invalid number of arguments
    if (argc != 2)
    {
        printf("Usage: %s <port>\n", argv[0]);
        exit(1);
    }
  
    pthread_mutex_init(&mutex, NULL);
  
    // create a TCP socket
    server_socket = socket(PF_INET, SOCK_STREAM, 0);  
  
    // server settings
    memset(&server_address, 0, sizeof(server_address));
    server_address.sin_family = AF_INET;
    server_address.sin_addr.s_addr = htonl(INADDR_ANY);
    server_address.sin_port = htons(atoi(argv[1]));
  
    // bind server's address information and server socket
    if (bind(server_socket, (struct sockaddr*)&server_address, sizeof(server_address)) == -1)
        error_handler("bind() error");
  
    // convert the server socket to listening socket
    if (listen(server_socket, 5) == -1)
        error_handler("listen() error");
        
    printf("Group chat server (port: %s)\n", argv[1]);
  
    while (1)
    {
        //size of Internet socket address structure
        client_address_size = sizeof(client_address);
  
        // accept client connection
        client_socket = accept(server_socket, (struct sockaddr*)&client_address, &client_address_size);
  
        // add the new client to the list
        pthread_mutex_lock(&mutex);  
        list_of_clients[number_of_clients++] = client_socket;  
        pthread_mutex_unlock(&mutex);
  
        // create thread
        pthread_create(&thread_id, NULL, receive_message, (void*)&client_socket); 
  
        // detach the thread
        pthread_detach(thread_id); 
  
        printf("Connected client IP: %s\n", inet_ntoa(client_address.sin_addr));
    }
  
    close(server_socket);
     
    return 0;
}
  
// receive client's message
void* receive_message(void *arg)
{
    int client_socket = *((int*)arg);
    int str_len = 0, i;
    char msg[MSG_LEN];
  
    while ((str_len = read(client_socket, msg, sizeof(msg))) != 0)
        send_message(msg, str_len);
      
    pthread_mutex_lock(&mutex);
  
    // before terminating the thread, remove itself (disconnected client) from the list
    for (i = 0; i < number_of_clients; i++)    
    {
        if (client_socket == list_of_clients[i])
        {
            while (i < number_of_clients - 1)
            {
                list_of_clients[i] = list_of_clients[i + 1];
                i++;
            }
  
            break;
        }
    }
      
    number_of_clients--;
    pthread_mutex_unlock(&mutex); 
  
    // terminate the connection
    close(client_socket);
  
    return NULL;
}
  
// send message to all connected clients
void send_message(char *msg, int len)
{
    int i;
  
    pthread_mutex_lock(&mutex);
  
    for (i = 0; i < number_of_clients; i++)
        write(list_of_clients[i], msg, len);
  
    pthread_mutex_unlock(&mutex);
}
  

void error_handler(char *err_msg)
{
    fputs(err_msg, stderr);
    fputc('\n', stderr);
    exit(1);
}