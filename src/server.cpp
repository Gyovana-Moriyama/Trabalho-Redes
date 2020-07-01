#include <iostream>
#include <pthread.h>
#include <signal.h>
#include "server.hpp"
#include "socket.hpp"

using namespace std;

//Struct to store Clients in a double linked list
struct s_clientList
{
    int socket;
    bool received;
    int attempts;
    ClientList *prev;
    ClientList *next;
    char ip[16];
    char name[NICKNAME_SIZE];
    bool isAdmin;
    bool muted;
    // char **channels;
    char channels[MAX_CHANNELS][CHANNEL_NAME_SIZE];
    ChannelList *activeChannel;
    ClientList *activeInstance;
    ClientList *mainNode;
};

typedef struct s_sendInfo
{
    ClientList *node;
    char *message;
} SendInfo;

typedef struct s_threadInfo
{
    ClientList *clientRoot;
    ClientList *clientNode;
    ChannelList *channelRoot;
} ThreadInfo;

struct s_channelList
{
    ChannelList *prev;
    ChannelList *next;
    char name[CHANNEL_NAME_SIZE];
    ClientList *clients;
};

ClientList *createClient(int sock_fd, char *ip)
{
    ClientList *newNode = (ClientList *)malloc(sizeof(ClientList));
    newNode->socket = sock_fd;
    newNode->received = false;
    newNode->attempts = 0;
    newNode->prev = NULL;
    newNode->next = NULL;
    strcpy(newNode->ip, ip);
    strcpy(newNode->name, "\0");
    newNode->isAdmin = false;
    newNode->muted = false;
    newNode->activeChannel = NULL;
    newNode->mainNode = NULL;

    // newNode->channels = (char **)malloc(sizeof(char *) * MAX_CHANNELS);
    for (int i = 0; i < MAX_CHANNELS; i++)
    {
        // newNode->channels[i] = (char *)malloc(sizeof(char) * CHANNEL_NAME_SIZE);
        newNode->channels[i][0] = '\0';
    }

    return newNode;
}

ChannelList *createChannelNode(char *name, ClientList *root)
{
    ChannelList *newNode = (ChannelList *)malloc(sizeof(ChannelList));

    newNode->prev = NULL;
    newNode->next = NULL;
    strcpy(newNode->name, name);
    newNode->clients = root;

    return newNode;
}

void printNode(ClientList *node)
{
    cout << "\n\tPrintando node: " << endl;
    if (node->prev == NULL)
        cout << "Node prev = NULL\n";
    else
        cout << "Node prev = " << node->prev->name << endl;

    cout << "Node = " << node->name << endl;

    if (node->next == NULL)
        cout << "Node next = NULL\n";
    else
        cout << "Node next = " << node->next->name << endl;
}

void disconnectNode(ClientList *node)
{
    cout << "Disconnecting " << node->name << endl;
    close(node->socket);
    if (node->next == NULL)
    {
        node->prev->next = NULL;
    }
    //remove a middle node
    else
    {
        node->prev->next = node->next;
        node->next->prev = node->prev;
    }

    free(node);
}

void deleteChannel(ChannelList *node)
{
    cout << "Deleting " << node->name << endl;

    if (node->next == NULL)
    {
        node->prev->next = NULL;
    }
    //remove a middle node
    else
    {
        node->prev->next = node->next;
        node->next->prev = node->prev;
    }

    free(node);
}

void errorMsg(const char *msg)
{
    perror(msg);
    exit(1);
}

void ctrl_c_handler(int sig)
{
    cout << "To exit use /quit" << endl;
}

void str_trim(char *str, char newchar)
{
    int n = strlen(str);
    //if the newchar is already at the end of the string, does not need to put
    if (newchar == str[n - 1])
    {
        return;
    }
    //is the last character of the string is a alphabet letter, put the newchar after it
    if (isalnum(str[n - 1]) != 0)
        str[n] = newchar;
    //if is not, substitue the last character
    else
        str[n - 1] = newchar;
}

void *quitHandler(void *rootNode)
{
    while (true)
    {
        char input[MESSAGE_SIZE];
        cin >> input;

        if (!strcmp(input, "/quit"))
        {
            ClientList *root = (ClientList *)rootNode;
            ClientList *tmp;
            while (root != NULL)
            {
                cout << "\nClose socketfd: " << root->socket << endl;
                close(root->socket);
                tmp = root;
                root = root->next;
                free(tmp);
            }
            cout << "Closing server...\n";
            exit(0);
        }
        else
        {
            cout << "Unknown command\n"
                 << "\tCommand List:\nQuit: /quit\n";
        }
    }
}

void send(ClientList *node, char message[])
{
    pthread_t sendThread;
    SendInfo *sendInfo = (SendInfo *)malloc(sizeof(SendInfo));
    sendInfo->node = node;
    sendInfo->message = (char *)malloc(sizeof(char) * (MESSAGE_SIZE + NICKNAME_SIZE + CHANNEL_NAME_SIZE + 5));
    strcpy(sendInfo->message, message);

    if (pthread_create(&sendThread, NULL, sendMessage, (void *)sendInfo) != 0)
    {
        errorMsg("Create thread error");
    }
    pthread_detach(sendThread);
}

void *sendMessage(void *info)
{
    SendInfo *sendInfo = (SendInfo *)info;
    int snd;

    do
    {
        snd = send(sendInfo->node->socket, sendInfo->message, MESSAGE_SIZE + NICKNAME_SIZE + CHANNEL_NAME_SIZE + 5, 0);
        sendInfo->node->mainNode->attempts++;
        usleep(WAIT_ACK);
    } while (snd >= 0 && !sendInfo->node->mainNode->received && sendInfo->node->mainNode->attempts < 5);

    // Disconnects client
    if (sendInfo->node->mainNode->attempts == 5 || snd < 0)
    {
        //Remove Node
        disconnectNode(sendInfo->node);
    }
    else if (sendInfo->node->mainNode->received)
    {
        sendInfo->node->mainNode->received = false;
        sendInfo->node->mainNode->attempts = 0;
        free(sendInfo->message);
        free(sendInfo);
    }
}

void sendAllClients(ClientList *root, ClientList *node, char message[])
{
    ClientList *tmp = root->next;
    int snd;

    while (tmp != NULL)
    {
        //sends the message to all clients except itself
        if (node->socket != tmp->socket)
        {
            cout << "Send to: " << tmp->name << " >> " << message;

            pthread_t sendThread;
            SendInfo *sendInfo = (SendInfo *)malloc(sizeof(SendInfo));
            sendInfo->node = tmp;
            sendInfo->message = (char *)malloc(sizeof(char) * (MESSAGE_SIZE + NICKNAME_SIZE + CHANNEL_NAME_SIZE + 5));
            strcpy(sendInfo->message, message);

            if (pthread_create(&sendThread, NULL, sendMessage, (void *)sendInfo) != 0)
                errorMsg("Create thread error");
            pthread_detach(sendThread);
        }
        tmp = tmp->next;
    }
}

void join(char *channel, ChannelList *root, ClientList *client)
{
    ChannelList *tmp = root;
    bool createNewChannel = true;

    // At the end of this loop, tmp will contain a pointer to the last node of the list, or to the existing channel
    while (tmp->next != NULL)
    {
        // If it finds an existing channel with the same name, sets a flag indicating that there is
        // no need to create a new channel, and then updates the pointer to that channel
        if (!strcmp(channel, tmp->next->name))
        {
            createNewChannel = false;
            tmp = tmp->next;
            break;
        }

        tmp = tmp->next;
    }

    if (createNewChannel)
    {
        // Creates secondary node of client
        ClientList *newClient = createClient(client->socket, client->ip);
        newClient->mainNode = client->mainNode;
        strcpy(newClient->name, client->mainNode->name);
        newClient->isAdmin = true;

        // Creates new channel and starts the list of clients, using an empty root node and the admin
        ClientList *rootNode = createClient(0, "0");
        ChannelList *newChannel = createChannelNode(channel, rootNode);
        tmp->next = newChannel;
        newChannel->prev = tmp;

        //Insert admin on the list
        newChannel->clients->next = newClient;
        newClient->prev = newChannel->clients;

        // Sets the new channel as the active one
        client->mainNode->activeChannel = newChannel;
        client->mainNode->activeInstance = newClient;

        //TODO verificar se atingiiu o numero maimo de canais
        // Searches for a empty space on the list of channels to put the new one
        for (int i = 0; i < MAX_CHANNELS; i++)
        {
            if (client->channels[i][0] == '\0')
            {
                strcpy(client->mainNode->channels[i], channel);
                break;
            }
        }

        char message[MESSAGE_SIZE];
        sprintf(message, "/channel %s Created and switched to channel", channel);
        send(client->mainNode, message);
    }
    else
    {
        // Checks if the user is already on that channel. If so, just changes the
        bool isInChannel = false;
        for (int i = 0; i < MAX_CHANNELS; i++)
        {
            if (!strcmp(client->channels[i], tmp->name))
            {
                isInChannel = true;
                client->mainNode->activeChannel = tmp;

                // Updates active instance
                ClientList *instance = tmp->clients;
                while (instance != NULL)
                {
                    if (!strcmp(instance->name, client->name))
                    {
                        client->mainNode->activeInstance = instance;
                        break;
                    }

                    instance = instance->next;
                }

                char message[MESSAGE_SIZE];
                sprintf(message, "/channel %s Switched to channel", channel);
                send(client->mainNode, message);

                break;
            }
        }

        if (!isInChannel)
        {
            // Creates an new client instance
            ClientList *newClient = createClient(client->socket, client->ip);
            newClient->mainNode = client->mainNode;
            strcpy(newClient->name, client->mainNode->name);

            // Adds the new client to the list (after the admin, who is client->next)
            newClient->next = tmp->clients->next->next;
            newClient->prev = tmp->clients->next;
            if(tmp->clients->next->next != NULL)
                tmp->clients->next->next->prev = newClient;
            tmp->clients->next->next = newClient;

            // Sets the channel as active one
            client->mainNode->activeChannel = tmp;
            client->mainNode->activeInstance = newClient;

            // Searches for a empty space on the list of channels to put the new one
            for (int i = 0; i < MAX_CHANNELS; i++)
            {
                if (client->mainNode->channels[i][0] == '\0')
                {
                    strcpy(client->mainNode->channels[i], channel);
                    break;
                }
            }

            char message[MESSAGE_SIZE];
            sprintf(message, "/channel %s You joined the channel", channel);
            send(newClient->mainNode, message);

            sprintf(message, "%s - %s joined the channel.\n", channel, newClient->name);
            sendAllClients(tmp->clients, newClient->mainNode, message);
        }
    }
}

bool whoIs(ClientList *admin, char *username)
{
    char buffer[MESSAGE_SIZE] = {};
    ClientList *tmp = admin->mainNode->activeChannel->clients->next;

    while (tmp != NULL)
    {
        if (!strcmp(tmp->name, username))
        {
            //send to admin the IP osf the user
            sprintf(buffer, "User(%s): %s\n", username, tmp->ip);
            int snd = send(admin->socket, buffer, MESSAGE_SIZE, 0);
            if (snd < 0)
                return false;
        }

        tmp = tmp->next;
    }

    return true;
}

void mute(ClientList *admin, char *username, bool mute)
{
    ClientList *tmp = admin->mainNode->activeChannel->clients->next;
    char message[MESSAGE_SIZE] = {};

    while (tmp != NULL)
    {
        if (!strcmp(tmp->name, username))
        {
            tmp->muted = mute;
            if (mute)
            {
                sprintf(message, "You were muted by %s.\n", admin->name);
                send(tmp->mainNode, message);
            }
            else
            {
                sprintf(message, "You were unmuted by %s.\n", admin->name);
                send(tmp->mainNode, message);
            }

            return;
        }
        tmp = tmp->next;
    }
}

void kick(ChannelList *root, ClientList *admin, char *username)
{
    ClientList *tmp = admin->mainNode->activeChannel->clients->next->next;

    while (tmp != NULL)
    {
        if (!strcmp(tmp->name, username))
        {
            // Removes node from channel's client list
            tmp->prev->next = tmp->next;
            if (tmp->next != NULL)
                tmp->next->prev = tmp->prev;

            // Removes channel from channel list
            for (int i = 0; i < MAX_CHANNELS; i++)
            {
                if (!strcmp(tmp->mainNode->channels[i], admin->mainNode->activeChannel->name))
                {
                    tmp->mainNode->channels[i][0] = '\0';
                    break;
                }
            }

            char message[MESSAGE_SIZE];
            sprintf(message, "/channel #none You were kicked out of the channel %s by %s. Switched to", admin->mainNode->activeChannel->name, admin->name);
            send(tmp->mainNode, message);

            sprintf(message, "%s - %s were kicked out of the channel.\n", admin->mainNode->activeChannel->name, tmp->name);
            sendAllClients(admin->mainNode->activeChannel->clients, tmp->mainNode, message);

            // Resets active channel and instance
            tmp->mainNode->activeChannel = root;
            tmp->mainNode->activeInstance = tmp->mainNode;

            free(tmp);
            return;
        }
        tmp = tmp->next;
    }
}

void *clientHandler(void *info)
{
    int leave_flag = 0;
    char recvBuffer[MESSAGE_SIZE] = {};
    char sendBuffer[MESSAGE_SIZE + NICKNAME_SIZE + CHANNEL_NAME_SIZE + 5] = {};
    ThreadInfo *tInfo = (ThreadInfo *)info;
    char message[MESSAGE_SIZE] = {};
    char command[MESSAGE_SIZE] = {};
    char argument[MESSAGE_SIZE] = {};

    //Announces the client that joined the chatroom
    cout << tInfo->clientNode->name << " (" << tInfo->clientNode->ip << ")"
         << " (" << tInfo->clientNode->socket << ")"
         << " joined the server.\n";
    sprintf(sendBuffer, "Server: %s joined the server.\n", tInfo->clientNode->name);
    sendAllClients(tInfo->clientRoot, tInfo->clientNode, sendBuffer);

    //Conversation
    while (true)
    {
        if (leave_flag)
            break;

        bzero(recvBuffer, MESSAGE_SIZE);
        bzero(sendBuffer, MESSAGE_SIZE + NICKNAME_SIZE + CHANNEL_NAME_SIZE + 5);
        bzero(command, MESSAGE_SIZE);
        bzero(argument, MESSAGE_SIZE);

        int rcv = recv(tInfo->clientNode->socket, recvBuffer, MESSAGE_SIZE, 0);

        if (rcv <= 0)
        {
            leave_flag = 1;
            continue;
        }

        if (recvBuffer[0] == '/')
        {
            sscanf(recvBuffer, "%s %s", command, argument);

            str_trim(command, '\0');
            str_trim(argument, '\0');

            if (!strcmp(command, "/ack"))
            {
                tInfo->clientNode->received = true;
                tInfo->clientNode->attempts = 0;
                cout << tInfo->clientNode->name << " received the message" << endl;
            }
            //sends that the client is quitting the chatroom
            else if (!strcmp(command, "/quit"))
            {
                cout << tInfo->clientNode->name << " (" << tInfo->clientNode->ip << ")"
                     << " (" << tInfo->clientNode->socket << ")"
                     << " left the server.\n";
                sprintf(sendBuffer, "Server: %s left the server.\n", tInfo->clientNode->name);
                sendAllClients(tInfo->clientRoot, tInfo->clientNode, sendBuffer);
                leave_flag = 1;
            }
            //if client sent /ping, the server answers with pong
            else if (!strcmp(command, "/ping"))
            {
                sprintf(sendBuffer, "Server: pong\n");
                send(tInfo->clientNode, sendBuffer);
            }
            else if (!strcmp(command, "/join"))
            {
                if (argument[0] == '&' || argument[0] == '#')
                {
                    join(argument, tInfo->channelRoot, tInfo->clientNode);
                }
                else
                {

                    sprintf(message, "Incorrect name form. Channel name needs to start with '#' or '&'.\n.");
                    send(tInfo->clientNode, message);
                }
            }
            else if (!strcmp(command, "/whois"))
            {
                if (tInfo->clientNode->activeInstance->isAdmin)
                {
                    if (!whoIs(tInfo->clientNode->activeInstance, argument))
                    {
                        leave_flag = 1;
                    }
                }
                else
                {
                    sprintf(message, "Invalid command. You are not the admin of this channel.\n");
                    send(tInfo->clientNode->activeInstance, message);
                }
            }
            else if (!strcmp(command, "/kick"))
            {
                if (tInfo->clientNode->activeInstance->isAdmin)
                {
                    kick(tInfo->channelRoot, tInfo->clientNode->activeInstance, argument);
                }
                else
                {
                    sprintf(message, "Invalid command. You are not the admin of this channel.\n");
                    send(tInfo->clientNode->activeInstance, message);
                }
            }
            else if (!strcmp(command, "/mute"))
            {
                if (tInfo->clientNode->activeInstance->isAdmin)
                {
                    mute(tInfo->clientNode->activeInstance, argument, true);
                }
                else
                {
                    sprintf(message, "Invalid command. You are not the admin of this channel.\n");
                    send(tInfo->clientNode->activeInstance, message);
                }
            }
            else if (!strcmp(command, "/unmute"))
            {
                if (tInfo->clientNode->activeInstance->isAdmin)
                {
                    mute(tInfo->clientNode->activeInstance, argument, false);
                }
                else
                {
                    sprintf(message, "Invalid command. You are not the admin of this channel.\n");
                    send(tInfo->clientNode->activeInstance, message);
                }
            }
        }
        else if (!tInfo->clientNode->activeInstance->muted)
        {
            sprintf(sendBuffer, "%s - %s: %s", tInfo->clientNode->activeChannel->name, tInfo->clientNode->name, recvBuffer);
            sendAllClients(tInfo->clientNode->activeChannel->clients, tInfo->clientNode->activeInstance, sendBuffer);
        }
    }

    //Remove node
    disconnectNode(tInfo->clientNode);
}

int main(int argc, char const *argv[])
{
    int server_fd = 0, client_fd = 0;
    int opt = 1;
    char nickname[NICKNAME_SIZE] = {};

    signal(SIGINT, ctrl_c_handler);

    //Create socket file descriptor
    if ((server_fd = socket(AF_INET, SOCK_STREAM, PROTOCOL)) < 0)
        errorMsg("Socket failed.");

    //Forcefully attaching socket to the port 8080
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt)))
        errorMsg("setsockopt");

    struct sockaddr_in server_addr, client_addr;
    int s_addrlen = sizeof(server_addr);
    int c_addrlen = sizeof(client_addr);
    bzero((char *)&server_addr, s_addrlen);
    bzero((char *)&client_addr, c_addrlen);

    //setup the host_addr structure for use in bind call
    //server byte order
    server_addr.sin_family = AF_INET;
    //automatically be filled with current host's IP address
    server_addr.sin_addr.s_addr = INADDR_ANY;
    //convert short int value from host to network byte order
    server_addr.sin_port = htons(PORT);

    //Bind the socket to the current IP address on port
    if (bind(server_fd, (struct sockaddr *)&server_addr, s_addrlen) < 0)
        errorMsg("bind failed");

    //Tells the socket to listen to the incoming connections
    //maximum size for the backlog queue is 5
    if (listen(server_fd, MAX_CONNECTIONS) < 0)
        errorMsg("listen");

    //Print server IP
    getsockname(server_fd, (struct sockaddr *)&server_addr, (socklen_t *)&s_addrlen);
    cout << "Start Server on: " << inet_ntoa(server_addr.sin_addr) << ": " << ntohs(server_addr.sin_port) << "\n";

    //Initial linked list for clients, the root is the server
    ClientList *clientRoot = createClient(server_fd, inet_ntoa(server_addr.sin_addr));

    ChannelList *channelRoot = createChannelNode("#server", clientRoot);

    //Thread to catch server input, in this case, is used to catch the '/quit'
    pthread_t inputThreadId;
    if (pthread_create(&inputThreadId, NULL, quitHandler, (void *)clientRoot) != 0)
        errorMsg("input thread ERROR");

    //Accepts new clients
    while (true)
    {

        if ((client_fd = accept(server_fd, (struct sockaddr *)&client_addr, (socklen_t *)&c_addrlen)) < 0)
        {
            cout << "Error accepting client" << endl;
            continue;
        }

        //Print client IP
        getpeername(client_fd, (struct sockaddr *)&client_addr, (socklen_t *)&c_addrlen);
        cout << "Client " << inet_ntoa(client_addr.sin_addr) << " : " << ntohs(client_addr.sin_port) << " joined.\n";

        //Create new client node and append to linked list
        ClientList *node = createClient(client_fd, inet_ntoa(client_addr.sin_addr));
        node->mainNode = node;
        node->activeInstance = node;
        node->muted = true;

        //Naming
        recv(node->socket, nickname, NICKNAME_SIZE, 0);
        strcpy(node->name, nickname);

        ClientList *last = clientRoot;
        while (last->next != NULL)
        {
            last = last->next;
        }

        node->prev = last;
        last->next = node;

        bzero(nickname, NICKNAME_SIZE);

        ThreadInfo *info = (ThreadInfo *)malloc(sizeof(ThreadInfo));
        info->clientRoot = clientRoot;
        info->clientNode = node;
        info->channelRoot = channelRoot;

        //create a new thread for each client
        pthread_t id;
        if (pthread_create(&id, NULL, clientHandler, (void *)info) != 0)
        {
            cout << "Create pthread error" << endl;

            //Remove Node
            disconnectNode(node);
            pthread_detach(id);
        }
    }
    return 0;
}