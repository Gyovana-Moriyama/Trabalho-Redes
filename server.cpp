#include <iostream>
#include <netinet/in.h>
#include <unistd.h>
#include <string.h>
#include <sys/socket.h>

#define PORT            8080
#define MESSAGE_SIZE    4096
#define PROTOCOL        0


using namespace std;

int main(int argc, char const *argv[]){
    int server_fd , new_socket, valread;
    int opt = 1;
    struct sockaddr_in serv_addr;
    int addrlen = sizeof(serv_addr);
    char buffer[MESSAGE_SIZE] = {0};
    char *hello = "Hello from server";

    if((server_fd = socket(AF_INET, SOCK_STREAM, PROTOCOL)) < 0){
        perror("Socket failed.");
        exit(EXIT_FAILURE);
    }

    if(setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt))){
        perror("setsockopt");
        exit(EXIT_FAILURE);
    }

    serv_addr.sin_family = AF_INET; 
    serv_addr.sin_addr.s_addr = INADDR_ANY; 
    serv_addr.sin_port = htons( PORT ); 

    if(bind(server_fd, (struct sockaddr *)&serv_addr, sizeof(serv_addr))<0){
        perror("bind failed");
        exit(EXIT_FAILURE);
    }

    if(listen(server_fd, 3) < 0){
        perror("listen");
        exit(EXIT_FAILURE);
    }


    if((new_socket = accept(server_fd, (struct sockaddr *)&serv_addr, (socklen_t*)&serv_addr))<0){
        perror("accept ERROR");
        exit(EXIT_FAILURE);
    }

    valread = read(new_socket, buffer, MESSAGE_SIZE);
    printf("%s\n", buffer);
    send(new_socket, hello, strlen(hello), 0);
    printf("Hello message sent\n");
    

    close(server_fd);
    close(new_socket);
    return 0;

}