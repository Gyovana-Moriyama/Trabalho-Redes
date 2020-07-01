#ifndef SERVER_H
#define SERVER_H

#define MAX_CONNECTIONS  5
#define MAX_CHANNELS     10

#define WAIT_ACK        400000

typedef struct s_channelList ChannelList;
typedef struct s_clientList ClientList;

/**
 * @brief  Creates new client node
 * @note   
 * @param  sock_fd: socket of the client
 * @param  *ip: IP of the client
 * @retval pointer to new client node
 */
ClientList *createClient(int sock_fd, char *ip);

/**
 * @brief  Creates new channel node
 * @note   
 * @param  *name: name of the channel
 * @param  *root: pointer to the root of the clients list
 * @retval pointer to new channel node
 */
ChannelList *createChannelNode(char *name, ClientList *root);


/**
 * @brief  Disconnects a specific client
 * @note   
 * @param  *node: pointer to the client to be disconnected
 * @param  *rootChannel: pointer to the root of the channels list
 * @retval None
 */
void disconnectNode(ClientList *node, ChannelList *rootChannel);

/**
 * @brief  Deletes a specific channel
 * @note   
 * @param  *node: pointer to the channel to be deleted
 * @retval None
 */
void deleteChannel(ChannelList *node);

/**
 * @brief  Put the newchar at the end of a string
 * @note   
 * @param  *str: string to be modified
 * @param  newchar: the new char that will be at he end of the string
 * @retval None
 */
void str_trim(char *str, char newchar);

/**
 * @brief  Closes the server
 * @note   
 * @param  *rootNode: pointer to the root of the list
 * @retval None
 */
void *quitHandler(void *info);

/**
 * @brief  Sends servers messages to a client
 * @note   
 * @param  *channelRoot: pointer to the root of the channels list 
 * @param  *node: pointer to the client that will receive the message
 * @param  message[]: the actual message
 * @retval None
 */
void send(ChannelList *channelRoot, ClientList *node, char message[]);

/**
 * @brief  Tries to send the message 5 times. If unsuccessful, disconnects client
 * @note   
 * @param  *info: pointer to the info struct that will be passed in the thread argument
 * @retval None
 */
void *sendMessage(void *info);

/**
 * @brief  Send the message to all clients
 * @note   
 * @param  *channelRoot: pointer to the root of the channels list 
 * @param  *root: pointer to the root of a clients list
 * @param  *node: pointer to the client hat is sending the message
 * @param  message[]: the actual message
 * @retval None
 */
void sendAllClients(ChannelList *channelRoot, ClientList *root, ClientList *node, char message[]);

/**
 * @brief  Creates new channel, switches client channel and client can join new channel
 * @note   
 * @param  *channel: name of the channel that the client will be joining
 * @param  *root: pointer to the root of the channels list
 * @param  *client: pointer to the client that will be joining the channel
 * @retval None
 */
void join(char *channel, ChannelList *root, ClientList *client);

/**
 * @brief  Tells the admin he IP of a specific user of the channel
 * @note   
 * @param  *admin: pointer to the admin of the channel
 * @param  *username:  name of he user that the admin is searching for
 * @retval returns true if success and false if seding message fails
 */
bool whoIs(ClientList *admin, char *username);

/**
 * @brief  Allows admin to mute/unmute a speciic user of the channel
 * @note   
 * @param  *root: pointer to the root of the channels list 
 * @param  *admin: pointer to the admin of the channel
 * @param  *username: name of he user that the admin is going to mute/unmute
 * @param  mute: bool: if true the user will be muted and if false unmuted
 * @retval None
 */
void mute(ChannelList *root, ClientList *admin, char *username, bool mute);

/**
 * @brief  Allows admin to kick a speciic user of the channel
 * @note   
 * @param  *root: poiner to the root nodeof the channels list
 * @param  *admin: pointer to the admin of the channel
 * @param  *username:  name of he user that the admin is going to kick
 * @retval None
 */
void kick(ChannelList *root, ClientList *admin, char *username);

/**
 * @brief  Handles the client
 * @note   
 * @param  *info: pointer to the info struct that will be passed in the thread argument
 * @retval None
 */
void *clientHandler(void *info);


#endif