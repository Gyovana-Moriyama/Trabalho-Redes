#include <iostream>
#include <netinet/in.h>
#include <unistd.h>
#include <string.h>
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

int main(int argc, char const *argv[]){

    int server_fd , new_socket, n;
    int opt = 1;
    struct sockaddr_in serv_addr;
    int addrlen = sizeof(serv_addr);
    char buffer[MESSAGE_SIZE] = {0};

    //Create socket file descriptor
    if((server_fd = socket(AF_INET, SOCK_STREAM, PROTOCOL)) < 0){
        perror("Socket failed.");
        exit(EXIT_FAILURE);
    }

    //Forcefully attaching socket to the port 8080
    if(setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt))){
        perror("setsockopt");
        exit(EXIT_FAILURE);
    }

    //setup the host_addr structure for use in bind call
    //server byte order
    serv_addr.sin_family = AF_INET; 
    //automatically be filled with current host's IP address
    serv_addr.sin_addr.s_addr = INADDR_ANY; 
    //convert short int value from host to network byte order
    serv_addr.sin_port = htons(PORT); 

    //Bind the socket to the current IP address on port
    if(bind(server_fd, (struct sockaddr *)&serv_addr, sizeof(serv_addr))<0){
        perror("bind failed");
        exit(EXIT_FAILURE);
    }

    //Tells the socket to listen to the incoming connections
    //maximum size for the backlog queue is 5
    if(listen(server_fd, 5) < 0){
        perror("listen");
        exit(EXIT_FAILURE);
    }

    //Actually accepts an incoming connection
    if((new_socket = accept(server_fd, (struct sockaddr *)&serv_addr, (socklen_t*)&addrlen))<0){
        perror("accept ERROR");
        exit(EXIT_FAILURE);
    }

    //loop comunication with the client
    while (1){
        //receives message
        bzero(buffer, MESSAGE_SIZE);
        n = recv(new_socket, buffer, MESSAGE_SIZE, 0);
        if(n < 0) errorMsg("ERROR reading from socket");
        cout << buffer << "\n";

        //sends message
        cout << "Please enter the message: ";
        bzero(buffer, MESSAGE_SIZE);
        fgets(buffer, MESSAGE_SIZE, stdin);
        n = send(new_socket, buffer, strlen(buffer), 0);
        if(n < 0) errorMsg("ERROR writing to socket");
        
    }


    close(server_fd);
    close(new_socket);
    return 0;

}