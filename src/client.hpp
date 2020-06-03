#ifndef CLIENT_H
#define CLIENT_H

#define DEFAULT_IP      "127.0.0.1"

//Closes client
void quit(int sock);

//Prints the user nickname before writes the message
void str_print_nickname();

//Put the newchar at the end of a string
void str_trim(char *str, char newchar);

//Prints the received message
void *receiveMsgHandler(void *sock);

// Gets input and send message to server
void *sendMsgHandler(void *sock);


#endif