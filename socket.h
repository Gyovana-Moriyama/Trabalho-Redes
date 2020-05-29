#ifndef SOCKET_H
#define SOCKET_H

#define PORT            8080
#define PROTOCOL        0

#define MESSAGE_SIZE    2049
#define NICKNAME_SIZE   10

typedef struct s_clientList ClientList;

void errorMsg(const char *msg);
void ctrl_c_handler(int sig);
ClientList *createNewNode(int server_fd, char *ip);

#endif