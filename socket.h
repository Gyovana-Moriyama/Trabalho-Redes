#ifndef SOCKET_H
#define SOCKET_H

#include <netinet/in.h>
#include <unistd.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>

#define PORT            8080
#define PROTOCOL        0

#define MESSAGE_SIZE    2049
#define NICKNAME_SIZE   10

#define DEFAULT_IP      "127.0.0.1"
#define MAX_CONNECTIONS  5

typedef struct s_clientList ClientList;

void errorMsg(const char *msg);
void ctrl_c_handler(int sig);
ClientList *createNewNode(int server_fd, char *ip);

#endif