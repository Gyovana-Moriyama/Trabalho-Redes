#ifndef SERVER_H
#define SERVER_H

#define MAX_CONNECTIONS  5

#define WAIT_ACK        400000

typedef struct s_clientList ClientList;

//Creates a new node
ClientList *createNewNode(int server_fd, char *ip);

//Disconnects a specific node
void disconnectNode(ClientList *node, ClientList *now);

//Closes the server
void *quitHandler(void *rootNode);

//Sends pong to the client that sent /ping
bool pong(ClientList *node, char message[]);

// Tries to send the message 5 times. If unsuccessful, disconnects client
void *sendMessage(void *info);

//Send the message to all clients
void sendAllClients(ClientList *root, ClientList *node, ClientList *now,char message[]);

//Handles the client
void *clientHandler(void *info);


#endif