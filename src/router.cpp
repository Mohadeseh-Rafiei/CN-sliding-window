#include <sys/socket.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <cstdlib>
#include <unistd.h>
#include <iostream>
#include <vector>
#include <arpa/inet.h>
#include "utility.h"
#include <fstream>
#include <pthread.h>

using namespace std;

unsigned int bufferSize;
Segment paket;
PacketACK ack;

// vector < char *> buffer;
// vector < char *> client_buffer;

struct UdpConnectionInfo
{
    struct sockaddr_in *clientAddr, *clientAddress, *serverAddr;
    unsigned int client_slen, server_slen;
    int client, server;
    vector<char *> *buffer;
    vector<char *> *client_buffer;
};

void die(char *s)
{
    perror(s);
    exit(1);
}

// void* receive(void * void_args) {
//     cout << "recieve" << endl;
//     UdpConnectionInfo *args = (UdpConnectionInfo*)void_args;
//
//     int server = ((UdpConnectionInfo *) void_args)->server;
//     int client = ((UdpConnectionInfo *) void_args)->client;
//     sockaddr_in *clientAddress = ((UdpConnectionInfo *) void_args)->clientAddress;
//     sockaddr_in *clientAddr = ((UdpConnectionInfo *) void_args)->clientAddr;
//     sockaddr_in *serverAddr = ((UdpConnectionInfo *) void_args)->serverAddr;
//     unsigned int client_slen = sizeof (serverAddr);
//     unsigned int server_slen = ((UdpConnectionInfo *) void_args)->server_slen;
//     vector<char*> *buffer = ((UdpConnectionInfo *) void_args)->buffer;
//     vector<char*> *client_buffer = ((UdpConnectionInfo *) void_args)->client_buffer;
//     int lastDataReceived = 0;
//     int lastAckReceived = 0;
//     cout << "vay" <<endl;
//     srand(time(NULL));
//     while(true){
//         cout << "while r" << endl;
//         char * recvBuf = (char *) &paket;
//         if (recvfrom(client, recvBuf, sizeof(paket), 0, (struct sockaddr*) &serverAddr, &client_slen) >= 0) {
//             cout << "---------------------------------" << endl;
//             cout << lastDataReceived << endl;
//             cout << recvBuf << endl;
//             lastDataReceived += 1;
//             client_buffer->push_back(recvBuf);
//         }
//         char* acksegment = (char *) &ack;
//         if (recvfrom(server, acksegment, sizeof(ack), 0, (struct sockaddr*) &clientAddress, &server_slen) >= 0) {
//             lastAckReceived += 1;
//             buffer->push_back(acksegment);
//         }
//
//     }
// }

// void* sent(void* void_args) {
//     cout << "send" << endl;
//
//     int server = ((UdpConnectionInfo *) void_args)->server;
//     int client = ((UdpConnectionInfo *) void_args)->client;
//     unsigned int client_slen = ((UdpConnectionInfo *) void_args)->client_slen;
//     sockaddr_in *clientAddress = ((UdpConnectionInfo *) void_args)->clientAddress;
//     sockaddr_in *clientAddr = ((UdpConnectionInfo *) void_args)->clientAddr;
//     unsigned int server_slen = ((UdpConnectionInfo *) void_args)->server_slen;
//     vector<char*> *buffer = ((UdpConnectionInfo *) void_args)->buffer;
//     vector<char*> *client_buffer = ((UdpConnectionInfo *) void_args)->client_buffer;
//
//     int lastDataSent = 0;
//     int lastAckSent = 0;
//
//     while(true) {
//         if(client_buffer->size()) {
//             char* segment = client_buffer->at(lastDataSent);
//             if (sendto(client, segment, sizeof(paket), 0, (struct sockaddr *) &clientAddress, client_slen) != -1) {
//                 lastDataSent += 1;
//             }
//         }
//         if(buffer->size()) {
//             char *sendBuf = buffer->at(lastAckSent);
//             if (sendto(server, sendBuf, sizeof(ack), 0, (struct sockaddr *) &clientAddr, server_slen) != -1) {
//                 lastAckSent += 1;
//             }
//         }
//     }
// }

void flush(bool *alreadyReceived)
{
    int i;
    for (i = 0; i < bufferSize; i++)
    {
        alreadyReceived[i] = false;
    }
}

int main(int argc, char *argv[])
{
    bufferSize = 256;
    int port = 8000;

    int serverSocket;
    struct sockaddr_in serverAddr, clientAddr;
    unsigned int slen = sizeof(serverAddr);

    if ((serverSocket = socket(AF_INET, SOCK_DGRAM, 0)) == -1)
    {
        die("socket");
    }

    struct timeval timeout;
    timeout.tv_sec = 0;
    timeout.tv_usec = 255000;

    //    cout << "avali " << serverSocket << " _ " << (char *) &timeout << " . " << sizeof(timeout) << endl;
    setsockopt(serverSocket, SOL_SOCKET, SO_RCVTIMEO, (char *)&timeout, sizeof(timeout));

    clientAddr.sin_family = AF_INET;
    clientAddr.sin_port = htons(port);
    clientAddr.sin_addr.s_addr = htonl(INADDR_ANY);

    if (bind(serverSocket, (struct sockaddr *)&clientAddr, sizeof(clientAddr)) == -1)
    {
        die("bind");
    }

    // client
    int client_port = 8001;

    int clientSocket;
    struct sockaddr_in clientAddress;

    if ((clientSocket = socket(AF_INET, SOCK_DGRAM, 0)) == 0)
    {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }
    memset((char *)&clientAddress, 0, sizeof(clientAddress));

    clientAddress.sin_family = AF_INET;
    clientAddress.sin_port = htons(client_port);
    clientAddress.sin_addr.s_addr = INADDR_ANY;

    struct timeval client_timeout;
    client_timeout.tv_sec = 0;
    client_timeout.tv_usec = 255000;

    //    cout << "dovomi " << clientSocket << " _ " << (char *) &client_timeout << " . " << sizeof(client_timeout);

    if (setsockopt(clientSocket, SOL_SOCKET, SO_RCVTIMEO, (char *)&client_timeout, sizeof(client_timeout)) < 0)
    {
        perror("setsockopt");
        exit(EXIT_FAILURE);
    }

    cout << endl
         << "threads are initialized!" << endl;
    cout << "-----------------------------------------------" << endl;
    fflush(stdout);

    pthread_t receive_t;
    pthread_t sent_t;

    struct UdpConnectionInfo client_t;
    client_t.client_slen = sizeof(clientAddr);
    client_t.server_slen = sizeof(clientAddress);
    client_t.clientAddr = &clientAddr;    // like receiver
    client_t.clientAddr = &clientAddress; // like sender
    client_t.serverAddr = &serverAddr;    // like sender
    client_t.client = clientSocket;
    client_t.server = serverSocket;
    vector<char *> buffer;
    vector<char *> client_buffer;
    client_t.buffer = &buffer;
    client_t.client_buffer = &client_buffer;

    // cout << "wtf" << endl;
    // cout << serverSocket << endl;
    //    pthread_create(&receive_t, NULL, receive, &client_t);
    //    pthread_create(&sent_t, NULL, sent, (void *)&client_t);
    int lastDataSent = 0;
    int lastAckSent = 0;
    int lastDataReceived = 0;
    int lastAckReceived = 0;

    while (true)
    {
        // cout << "while" <<endl;
        char *recvBuf = (char *)&paket;
        if (recvfrom(serverSocket, recvBuf, sizeof(paket), 0, (struct sockaddr *)&serverAddr, &client_t.client_slen) >= 0)
        {
            cout << "--------------------------------" << endl;
            cout << "data rec" << endl;
            cout << lastDataReceived << endl;
            lastDataReceived += 1;
            client_buffer.push_back(recvBuf);
        }
        char *acksegment = (char *)&ack;
        if (recvfrom(clientSocket, acksegment, sizeof(ack), 0, (struct sockaddr *)&clientAddress, &client_t.server_slen) >= 0)
        {
            cout << "r ack" << endl;
            lastAckReceived += 1;
            buffer.push_back(acksegment);
        }

        if (client_buffer.size())
        {
            char *segment = client_buffer[lastDataSent];
            if (sendto(clientSocket, segment, sizeof(paket), 0, (struct sockaddr *)&clientAddress, client_t.client_slen) != -1)
            {
                cout << "s data" << endl;
                lastDataSent += 1;
            }
        }
        if (buffer.size())
        {
            char *sendBuf = buffer[lastAckSent];
            if (sendto(serverSocket, sendBuf, sizeof(ack), 0, (struct sockaddr *)&serverAddr, client_t.server_slen) != -1)
            {
                cout << "sand ack" << endl;
                lastAckSent += 1;
            }
        }
    }
    return 0;
}