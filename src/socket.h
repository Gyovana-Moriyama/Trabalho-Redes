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

//Shows error message and exit
void errorMsg(const char *msg);

//Handle with Ctrl C
void ctrl_c_handler(int sig);


#endif