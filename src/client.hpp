#ifndef CLIENT_H
#define CLIENT_H

#define DEFAULT_IP      "127.0.0.1"

/**
 * @brief  Closes client
 * @note   
 * @param  sock: socket of the client
 * @retval None
 */
void quit(int sock);

/**
 * @brief  Prints the user nickname before writes the message
 * @note   
 * @retval None
 */
void str_print_nickname();


/**
 * @brief  Prints the received message
 * @note   
 * @param  *sock: socket of the client
 * @retval None
 */
void *receiveMsgHandler(void *sock);

/**
 * @brief  Gets input and send message to server
 * @note   
 * @param  *sock: socket of the client
 * @retval None
 */
void *sendMsgHandler(void *sock);


#endif