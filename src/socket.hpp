#ifndef SOCKET_H
#define SOCKET_H

#include <netinet/in.h>
#include <unistd.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>

#define PORT                8080
#define PROTOCOL            0

#define MESSAGE_SIZE        2049
#define NICKNAME_SIZE       51
#define CHANNEL_NAME_SIZE   201

/**
 * @brief  Shows error message and exit
 * @note   
 * @param  *msg: message tha will be printed
 * @retval None
 */
void errorMsg(const char *msg);

/**
 * @brief  Handle with Ctrl C
 * @note   
 * @param  sig: signal
 * @retval None
 */
void ctrl_c_handler(int sig);


#endif