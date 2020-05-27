#include <iostream>
#include <netinet/in.h>
#include <unistd.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <signal.h>
#include "socket.h"

using namespace std;

//Struct to store Clients in a double linked list
typedef struct s_clientList
{
    int data;
    struct s_clientList *prev;
    struct s_clientList *next;
    char ip[16];
    char name[NICKNAME_SIZE];

} ClientList;

//Shows error message and exit
void errorMsg(const char *msg)
{
    perror(msg);
    exit(1);
}

//Closes the server
//!Tem que implementar sair do server
void *quitHandler(void *rootNode)
{
    while (true)
    {
        string input;
        cin >> input;

        if (input.compare("/quit") == 0)
        {
            ClientList *root = (ClientList *)rootNode;
            ClientList *tmp;
            while (root != NULL)
            {
                cout << "\nClose socketfd: " << root->data << endl;
                close(root->data);
                tmp = root;
                root = root->next;
                free(tmp);
            }
            cout << "Closing server...\n";
            exit(0);
        }
        else
        {
            cout << "Unknown command\n" << "\tCommand List:\nQuit: /quit\n";
        }
    }
}

//!Sla o que eu faço com o handler de CRTL + C
void ctrl_c_handler(int sig)
{
    cout << "To exit use /quit" << endl;
}

//Creates a new node
ClientList *createNewNode(int server_fd, char *ip)
{
    ClientList *newNode = (ClientList *)malloc(sizeof(ClientList));
    newNode->data = server_fd;
    newNode->prev = NULL;
    newNode->next = NULL;
    strcpy(newNode->ip, ip);
    strcpy(newNode->name, "\0");

    return newNode;
}

//Send the message to all clients
void sendAllClients(ClientList *root, ClientList *node, char tmpBuffer[])
{
    ClientList *tmp = root->next;
    while (tmp != NULL)
    {
        //sends the message to all clients except itself
        if (node->data != tmp->data)
        {
            cout << "Send to: " << tmp->name << ">> " << tmpBuffer;
            send(tmp->data, tmpBuffer, MESSAGE_SIZE, 0);
        }
        tmp = tmp->next;
    }
}

//Sends pong to the client that sent /ping
void pong(ClientList *node, char tmpBuffer[])
{
    send(node->data, tmpBuffer, MESSAGE_SIZE, 0);
}

//Handles the client
void *clientHandler(void *info)
{
    int leave_flag = 0;
    char nickname[NICKNAME_SIZE] = {};
    char recvBuffer[MESSAGE_SIZE] = {};
    char sendBuffer[MESSAGE_SIZE] = {};
    ClientList *root = ((ClientList **)info)[0];
    ClientList *node = ((ClientList **)info)[1];
    ClientList *now = ((ClientList **)info)[2];

    //Naming
    if (recv(node->data, nickname, NICKNAME_SIZE, 0) <= 0)
    {
        cout << node->ip << "didn't input name.\n";
        leave_flag = 1;
    }
    else
    {
        strcpy(node->name, nickname);
        cout << node->name << "(" << node->ip << ")"
             << "(" << node->data << ")"
             << " join the chatroom.\n";
        sprintf(sendBuffer, "%s join the chatroom.\n", node->name);
        sendAllClients(root, node, sendBuffer);
    }

    //Conversation
    while (true)
    {
        if (leave_flag)
            break;

        bzero(recvBuffer, MESSAGE_SIZE);
        bzero(sendBuffer, MESSAGE_SIZE);

        int rcv = recv(node->data, recvBuffer, MESSAGE_SIZE, 0);

        if (rcv <= 0)
            errorMsg("ERROR reading from socket");

        //sends that the client is quitting the chatroom
        if (!strcmp(recvBuffer, "/quit"))
        {
            cout << node->name << "(" << node->ip << ")"
                 << "(" << node->data << ")"
                 << " leave the chatroom.\n";
            sprintf(sendBuffer, "%s leave the chatroom.\n", node->name);
            sendAllClients(root, node, sendBuffer);
            leave_flag = 1;
        }
        //if client sent /ping, the server answers with pong
        else if (!strcmp(recvBuffer, "/ping"))
        {
            sprintf(sendBuffer, "Server: pong");
            pong(node, sendBuffer);
        }
        else
        {
            sprintf(sendBuffer, "%s: %s", node->name, recvBuffer);
            sendAllClients(root, node, sendBuffer);
        }

    }

    //Remove Node
    close(node->data);
    if (node == now)
    {
        now = node->prev;
        now->next = NULL;
    }
    //remove a middle node
    else
    {
        node->prev->next = node->next;
        node->next->prev = node->prev;
    }

    free(node);
}

// void *receiveMsg(void *info){
//     int n;
//     char buffer[MESSAGE_SIZE];
//     int sock = ((int *)info)[0];
//     int server_fd = ((int *)info)[1];

//     while(true) {
//         // Receives message
//         bzero(buffer, MESSAGE_SIZE);
//         n = recv(sock, buffer, MESSAGE_SIZE, 0);
//         if(n <= 0) errorMsg("ERROR reading from socket");

//         // // If receives the quit command, closes server and quit
//         // if (!strcmp(buffer, "/quit\n")) {
//         //     cout << "Quitting";
//         //     quit(sock, server_fd);
//         // }

//         if(!strcmp(buffer, "/ping")){
//             // cout << "pong\n";
//             // sendMsg(info);
//         }
//         else if(!strcmp(buffer, "/connect")){
//             cout << "Client connected\n";
//         }
//         else{
//             cout << "Incoming >> " << buffer << "\n";

//         }
//     }
// }

// void *sendMsg(void *info) {
//     int n;
//     char buffer[MESSAGE_SIZE];
//     string message;

//     int sock = ((int *)info)[0];
//     int server_fd = ((int *)info)[1];

//     while(true){
//         // Gets input
//         getline(cin, message);

//         // Calculates how many parts the message will be divided into
//         int div = (message.length() > MESSAGE_SIZE) ? (message.length()/MESSAGE_SIZE) : 0;

//         for(int i=0; i<=div; i++) {
//             // Clear buffer
//             bzero(buffer, MESSAGE_SIZE);

//             // Copy message limited by MESSAGE_SIZE
//             message.copy(buffer, MESSAGE_SIZE, i*MESSAGE_SIZE);

//             // Sends message
//             n = send(sock, buffer, strlen(buffer), MSG_DONTWAIT);
//             if(n < 0) errorMsg("ERROR writing to socket");

//             // // Quits if receive quit message
//             // if (!strcmp(buffer, "/quit")) {
//             //     cout << "Quitting";
//             //     quit(sock, server_fd);
//             // }
//         }
//     }
// }

int main(int argc, char const *argv[])
{

    int server_fd = 0, client_fd = 0;
    int opt = 1;
    struct sockaddr_in server_addr, client_addr;
    int s_addrlen = sizeof(server_addr);
    int c_addrlen = sizeof(client_addr);

    signal(SIGINT, ctrl_c_handler);

    //Create socket file descriptor
    if ((server_fd = socket(AF_INET, SOCK_STREAM, PROTOCOL)) < 0)
    {
        errorMsg("Socket failed.");
    }

    //Forcefully attaching socket to the port 8080
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt)))
    {
        errorMsg("setsockopt");
    }

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
    {
        errorMsg("bind failed");
    }

    //Tells the socket to listen to the incoming connections
    //maximum size for the backlog queue is 5
    if (listen(server_fd, 5) < 0)
    {
        errorMsg("listen");
    }

    //Print server IP
    getsockname(server_fd, (struct sockaddr *)&server_addr, (socklen_t *)&s_addrlen);
    cout << "Start Server on: " << inet_ntoa(server_addr.sin_addr) << ":" << ntohs(server_addr.sin_port) << "\n";

    //Initial linked list for clients
    ClientList *root = createNewNode(server_fd, inet_ntoa(server_addr.sin_addr));
    ClientList *now = root;

    
    pthread_t inputThreadId;
    if (pthread_create(&inputThreadId, NULL, quitHandler, (void *)root) != 0)
    {
        errorMsg("input thread ERROR");
    }

    while (true)
    {
        client_fd = accept(server_fd, (struct sockaddr *)&client_addr, (socklen_t *)&c_addrlen);

        // if ((client_fd = accept(server_fd, (struct sockaddr *)&client_addr, (socklen_t *)&c_addrlen)) < 0)
        // {
        //     errorMsg("accept ERROR");
        // }

        //Print client IP
        getpeername(client_fd, (struct sockaddr *)&client_addr, (socklen_t *)&c_addrlen);
        cout << "Client " << inet_ntoa(client_addr.sin_addr) << " : " << ntohs(client_addr.sin_port) << " came in.\n";

        //Append linked list for clients
        ClientList *node = createNewNode(client_fd, inet_ntoa(client_addr.sin_addr));
        node->prev = now;
        now->next = node;
        now = node;

        ClientList *info[3] = {root, node, now};

        //create a new thread for each client
        pthread_t id;
        if (pthread_create(&id, NULL, clientHandler, (void *)&info) != 0)
        {
            errorMsg("Create pthread error\n");
        }
    }

    // //Actually accepts an incoming connection
    // if ((new_socket = accept(server_fd, (struct sockaddr *)&server_addr, (socklen_t *)&s_addrlen)) < 0)
    // {
    //     errorMsg("accept ERROR");
    // }

    // // Array with the informations that will be passed as argument to threads
    // int info[2] = {new_socket, server_fd};

    // Creates 2 threads. One used to receive messages, and other to send messages.
    // pthread_t threads[NUM_THREADS];

    // pthread_create(&threads[0], NULL, receiveMsg, info);
    // pthread_create(&threads[1], NULL, sendMsg, info);

    // Keeps threads running
    // while (true)
    //     ;

    return 0;
}