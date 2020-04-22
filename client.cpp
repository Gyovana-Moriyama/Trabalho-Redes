#include <iostream>
#include <string>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
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

void quit(int sock) {
    close(sock);
    exit(0);
}

void *receiveMsg(void *sock){
    int n;
    char buffer[MESSAGE_SIZE];

    while(true) {
        // Receives message
        bzero(buffer, MESSAGE_SIZE);
        n = recv(*(int *)sock, buffer, MESSAGE_SIZE, 0);
        if(n <= 0) errorMsg("ERROR reading from socket");

        // Quits if receive quit message
        if (!strcmp(buffer, "/quit\n")) {
            cout << "Quitting";
            quit(*(int *)sock);
        }

        cout << "Incoming >> " << buffer << "\n"; ; 
    }
}

void *sendMsg(void *sock) {
    int n;
    char buffer[MESSAGE_SIZE];
    string message;

    while(true){
        // Gets input
        getline(cin, message);

        // Calculates how many parts the message will be divided into
        int div = (message.length() > MESSAGE_SIZE) ? (message.length()/MESSAGE_SIZE) : 0;

        for(int i=0; i<=div; i++) {
            // Clear buffer
            bzero(buffer, MESSAGE_SIZE);

            // Copy message limited by MESSAGE_SIZE
            message.copy(buffer, MESSAGE_SIZE, i*MESSAGE_SIZE);

            // Sends message
            n = send(*(int *)sock, buffer, strlen(buffer), MSG_DONTWAIT);
            if(n < 0) errorMsg("ERROR writing to socket");

            // Quits if receive quit message
            if (!strcmp(buffer, "/quit")) {
                cout << "Quitting";
                quit(*(int *)sock);
            }
        }
    }
}


int main(int argc, char const *argv[]){
    int sock = 0;
    int n;
    struct sockaddr_in serv_addr;
    char ip[10] = "127.0.0.1";
    char buffer[MESSAGE_SIZE] = {0};

    //Create socket file descriptor
    if((sock = socket(AF_INET, SOCK_STREAM, PROTOCOL)) < 0){
        cout << "\nSocket creation error \n";
        return -1;
    }

    //setup the host_addr structure for use in bind call
    //server byte order
    serv_addr.sin_family = AF_INET;
    //convert short int value from host to network byte order
    serv_addr.sin_port = htons(PORT);

    // Convert IPv4 and IPv6 addresses from text to binary form 
    if(inet_pton(AF_INET, ip, &serv_addr.sin_addr) <= 0){
        cout << "\nInvalid address/ Address not supported\n";
        return -1;
    }

    //Connects to server
    if(connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0){
        cout << "\nConnection failed\n";
        return -1;
    }
    
    // Creates 2 threads. One used to receive messages, and other to send messages.
    pthread_t threads[NUM_THREADS];
    
    pthread_create(&threads[1], NULL, sendMsg, &sock);
    pthread_create(&threads[0], NULL, receiveMsg, &sock);

    // Keeps threads running
    while (true);
    
    return 0;
}