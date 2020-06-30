#include <iostream>
#include <pthread.h>
#include <signal.h>
#include "client.hpp"
#include "socket.hpp"

using namespace std;

//Global variables
char nickname[MESSAGE_SIZE] = {};

void errorMsg(const char *msg)
{
    perror(msg);
    exit(1);
}

void quit(int sock)
{
    close(sock);
    exit(0);
}

void ctrl_c_handler(int sig)
{
    cout << "To exit use /quit" << endl;
}

void str_print_nickname()
{
    printf("\r%s: ", nickname);
    fflush(stdout);
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

void *receiveMsgHandler(void *sock)
{
    char buffer[MESSAGE_SIZE + NICKNAME_SIZE + 2] = {};
    int rcv;
    while (true)
    {
        rcv = recv(*(int *)sock, buffer, MESSAGE_SIZE + NICKNAME_SIZE + 2, 0);
        if (rcv == 0)
        {
            cout << "\rLost connection to server...\n";
            fflush(stdout);
            quit(*(int *)sock);
        }
        else if (rcv < 0)
            errorMsg("ERROR reading from socket");
        else
        {
            cout << "\r" << buffer;
            fflush(stdout);
            str_print_nickname();

            // Sends message to server, informing that the message was received
            bzero(buffer, MESSAGE_SIZE + NICKNAME_SIZE + 2);
            strcpy(buffer, "/ack");
            int snd = send(*(int *)sock, buffer, strlen(buffer), MSG_DONTWAIT);
            if (snd < 0)
                errorMsg("ERROR writing to socket");
        }
    }
}

void *sendMsgHandler(void *sock)
{
    char buffer[MESSAGE_SIZE] = {};
    string message;
    int snd;
    while (true)
    {
        str_print_nickname();
        //  Gets input
        getline(cin, message);
        // Calculates how many parts the message will be divided into
        int div = (message.length() > (MESSAGE_SIZE - 1)) ? (message.length() / (MESSAGE_SIZE - 1)) : 0;

        for (int i = 0; i <= div; i++)
        {
            // Clear buffer
            bzero(buffer, MESSAGE_SIZE);

            // Copy message limited by MESSAGE_SIZE
            message.copy(buffer, MESSAGE_SIZE - 1, (i * (MESSAGE_SIZE - 1)));

            //Put \n at the end of the non command messages
            if (buffer[0] != '/')
                str_trim(buffer, '\n');
            // Sends message
            snd = send(*(int *)sock, buffer, strlen(buffer), MSG_DONTWAIT);
            if (snd < 0)
                errorMsg("ERROR writing to socket");
        }

        // Quits if receive quit message
        if (!strcmp(buffer, "/quit"))
        {
            cout << "Quitting\n";
            quit(*(int *)sock);
        }
    }
}

int main(int argc, char const *argv[])
{
    int n;
    struct sockaddr_in server_addr, client_addr;
    int s_addrlen = sizeof(server_addr);
    int c_addrlen = sizeof(client_addr);
    int sock = 0;
    char buffer[MESSAGE_SIZE] = {};
    char command[MESSAGE_SIZE] = {};


    signal(SIGINT, ctrl_c_handler);
    bzero(nickname, NICKNAME_SIZE);

    //Get nickname
    do
    {
        cout << "Please enter your nickname (1~50 characters): /nickname username: ";
        if (fgets(buffer, MESSAGE_SIZE - 1, stdin) != NULL)
        {
            if (buffer[0] == '/')
            {
                sscanf(buffer, "%s %s", command, nickname);
                if(!strcmp(command, "/nickname")){
                    str_trim(nickname, '\0');
                }
                else
                {
                    cout << "Invalid command.\n";
                }
                
            }
        }
    } while (strlen(nickname) < 1 || strlen(nickname) > NICKNAME_SIZE - 1);


    bzero(buffer, MESSAGE_SIZE);

    //Create socket file descriptor
    if ((sock = socket(AF_INET, SOCK_STREAM, PROTOCOL)) < 0)
        errorMsg("\nSocket creation error \n");

    bzero((char *)&server_addr, s_addrlen);
    bzero((char *)&client_addr, c_addrlen);

    //setup the host_addr structure for use in bind call
    //server byte order
    server_addr.sin_family = AF_INET;
    //convert short int value from host to network byte order
    server_addr.sin_port = htons(PORT);

    char ip[10] = {};

    //Gets server address, it can be the default or any other valid ip
    do
    {
        cout << "Please enter server address (default: " << DEFAULT_IP << ").\nFor default enter /default: ";
        if (fgets(ip, MESSAGE_SIZE - 1, stdin) != NULL)
            str_trim(ip, '\0');

    } while (strlen(ip) < 8 || strlen(ip) > 9);

    int addr = 0;

    if (!strcmp(ip, "/default"))
    {
        // Convert IPv4 and IPv6 addresses from text to binary form
        addr = inet_pton(AF_INET, DEFAULT_IP, &server_addr.sin_addr);
    }
    else
    {
        // Convert IPv4 and IPv6 addresses from text to binary form
        addr = inet_pton(AF_INET, ip, &server_addr.sin_addr);
    }

    if (addr <= 0)
        errorMsg("\nInvalid address/ Address not supported\n");

    //commands menu
    cout << "Connect: /connect\nQuit: /quit\nPing: /ping\n";
    bool connected = false;
    while (!connected)
    {
        if (fgets(buffer, 12, stdin) != NULL)
        {
            str_trim(buffer, '\0');
        }

        // sets 'connected' flag to true and exits loop
        if (!strcmp(buffer, "/connect"))
            connected = true;
        //quits without connect to server
        else if (!strcmp(buffer, "/quit"))
        {
            cout << "Quitting\n";
            quit(sock);
        }
        //can't do /ping if is no connected to server
        else if (!strcmp(buffer, "/ping"))
            cout << "/ping can only be used after connecting to server.\n";
        else
            cout << "Unknown command" << endl;
    }

    //Connects to server
    if (connect(sock, (struct sockaddr *)&server_addr, s_addrlen) < 0)
        errorMsg("\nConnection failed\n");

    //Names
    getsockname(sock, (struct sockaddr *)&client_addr, (socklen_t *)&c_addrlen);
    getsockname(sock, (struct sockaddr *)&server_addr, (socklen_t *)&s_addrlen);
    cout << "Connect to Server: " << inet_ntoa(server_addr.sin_addr) << ": " << ntohs(server_addr.sin_port) << "\n";
    cout << "You are: " << inet_ntoa(client_addr.sin_addr) << ": " << ntohs(client_addr.sin_port) << "\n";

    send(sock, nickname, NICKNAME_SIZE, 0);

    pthread_t recvMsgThread;
    if (pthread_create(&recvMsgThread, NULL, receiveMsgHandler, &sock) != 0)
    {
        errorMsg("\nCreate pthread error\n");
    }

    pthread_t sendMsgThread;
    if (pthread_create(&sendMsgThread, NULL, sendMsgHandler, &sock) != 0)
        errorMsg("\nCreate pthread error\n");

    // Keeps threads running
    pthread_join(recvMsgThread, NULL);
    pthread_join(sendMsgThread, NULL);

    return 0;
}