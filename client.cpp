#include <iostream>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>

#define PORT            8080
#define MESSAGE_SIZE    4096
#define PROTOCOL        0

using namespace std;

//Shows error message and exit
void errorMsg(const char *msg){
    perror(msg);
    exit(0);
}

int main(int argc, char const *argv[])
{
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
    
    //loop comunication with the client
    while (1){
        //Sends message
        cout << "Please enter the message: ";
        bzero(buffer, MESSAGE_SIZE);
        fgets(buffer, MESSAGE_SIZE, stdin);
        n = send(sock, buffer, strlen(buffer), 0);
        if(n < 0) errorMsg("ERROR writing to socket");

        //receives message
        bzero(buffer, MESSAGE_SIZE);
        n = recv(sock, buffer, MESSAGE_SIZE, 0);
        if(n < 0) errorMsg("ERROR reading from socket");
        cout << buffer << "\n";
           
    }
    


    close(sock);
    return 0;
}
