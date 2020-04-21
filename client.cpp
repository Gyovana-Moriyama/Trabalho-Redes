#include <iostream>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>

#define PORT            8080
#define MESSAGE_SIZE    4096
#define PROTOCOL        0

using namespace std;

int main(int argc, char const *argv[])
{
    int sock = 0;
    int valread;
    struct sockaddr_in serv_addr;
    char *hello = "Hello from client";
    char *ip = "127.0.0.1";
    char buffer[MESSAGE_SIZE] = {0};

    if((sock = socket(AF_INET, SOCK_STREAM, PROTOCOL)) < 0){
        printf("\nSocket creation error \n");
        return -1;
    }

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT);

    if(inet_pton(AF_INET, ip, &serv_addr.sin_addr) <= 0){
        printf("\nInvalid address/ Address not supported\n");
        return -1;
    }

    if(connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0){
        printf("\nConnection failed\n");
        return -1;
    }
    


    send(sock, hello, strlen(hello), 0);
    printf("Hello message sent\n");
    valread = read(sock, buffer, MESSAGE_SIZE);
    printf("%s\n", buffer);

    close(sock);
    return 0;
}
