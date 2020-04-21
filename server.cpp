#include <iostream>
#include <netinet/in.h>
#include <unistd.h>
#include <string.h>
#include <sys/socket.h>
#include <pthread.h>

#define PORT            8080
#define MESSAGE_SIZE    4096
#define PROTOCOL        0

#define NUM_THREADS     2

using namespace std;

//Shows error message and exit
void errorMsg(const char *msg){
    perror(msg);
    exit(1);
}

void quit(int sock, int server_fd) {
    close(sock);
    close(server_fd);
    exit(0);
}

void *receiveMsg(void *info){
    int n;
    char buffer[MESSAGE_SIZE];
    int sock = ((int *)info)[0];
    int server_fd = ((int *)info)[1];

    while(true) {
        // Receives message
        bzero(buffer, MESSAGE_SIZE);
        n = recv(sock, buffer, MESSAGE_SIZE, 0);
        if(n <= 0) errorMsg("ERROR reading from socket");

        // If receives the quit command, closes server and quit
        if (!strcmp(buffer, "/quit\n")) {
            cout << "Quitting";
            quit(sock, server_fd);
        }

        cout << "Incoming >> " << buffer; 
    }
}

void *sendMsg(void *info) {
    int n;
    char buffer[MESSAGE_SIZE];

    int sock = ((int *)info)[0];
    int server_fd = ((int *)info)[1];

    while(true){
        // Gets input
        bzero(buffer, MESSAGE_SIZE);
        fgets(buffer, MESSAGE_SIZE, stdin);

        // Sends message
        n = send(sock, buffer, strlen(buffer), MSG_DONTWAIT);
        if(n < 0) errorMsg("ERROR writing to socket");

        // If receives the quit command, closes server and quit
        if (!strcmp(buffer, "/quit\n")) {
            cout << "Quitting";
            quit(sock, server_fd);
        }
    }
}

int main(int argc, char const *argv[]){
    int server_fd , new_socket, n;
    int opt = 1;
    struct sockaddr_in serv_addr;
    int addrlen = sizeof(serv_addr);
    char buffer[MESSAGE_SIZE] = {0};

    //Create socket file descriptor
    if((server_fd = socket(AF_INET, SOCK_STREAM, PROTOCOL)) < 0){
        errorMsg("Socket failed.");
    }

    //Forcefully attaching socket to the port 8080
    if(setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt))){
        errorMsg("setsockopt");
    }

    //setup the host_addr structure for use in bind call
    //server byte order
    serv_addr.sin_family = AF_INET; 
    //automatically be filled with current host's IP address
    serv_addr.sin_addr.s_addr = INADDR_ANY; 
    //convert short int value from host to network byte order
    serv_addr.sin_port = htons(PORT); 

    //Bind the socket to the current IP address on port
    if(bind(server_fd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0){
        errorMsg("bind failed");
    }

    //Tells the socket to listen to the incoming connections
    //maximum size for the backlog queue is 5
    if(listen(server_fd, 5) < 0){
        errorMsg("listen");
    }

    //Actually accepts an incoming connection
    if((new_socket = accept(server_fd, (struct sockaddr *)&serv_addr, (socklen_t*)&addrlen)) < 0){
        errorMsg("accept ERROR");
    }

    // Array with the informations that will be passed as argument to threads
    int info[2] = { new_socket, server_fd };

    // Creates 2 threads. One used to receive messages, and other to send messages.
    pthread_t threads[NUM_THREADS]; 

    pthread_create(&threads[0], NULL, receiveMsg, info);
    pthread_create(&threads[1], NULL, sendMsg, info);

    // Keeps threads running
    while (true);
    
    return 0;
}