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
    int *lastDataSent;
    int *lastDataReceived;
    int *lastAckReceived;
    int *lastAckSent;
};

void die(char *s)
{
    perror(s);
    exit(1);
}

 void* receiveData(void * void_args) {
     cout << "recieve Data" << endl;
//     UdpConnectionInfo *args = (UdpConnectionInfo*)void_args;

     int server = ((UdpConnectionInfo *) void_args)->server;
     int client = ((UdpConnectionInfo *) void_args)->client;
     sockaddr_in *clientAddress = ((UdpConnectionInfo *) void_args)->clientAddress;
     sockaddr_in *clientAddr = ((UdpConnectionInfo *) void_args)->clientAddr;
     sockaddr_in *serverAddr = ((UdpConnectionInfo *) void_args)->serverAddr;
     unsigned int client_slen = sizeof(*serverAddr);
     unsigned int server_slen = ((UdpConnectionInfo *) void_args)->server_slen;
     vector<char*> *buffer = ((UdpConnectionInfo *) void_args)->buffer;
     vector<char*> *client_buffer = ((UdpConnectionInfo *) void_args)->client_buffer;
//     int *lastDataReceived = ((UdpConnectionInfo *) void_args)->lastDataReceived;

     cout << "vay" <<endl;
     srand(time(NULL));
     while(true){
         cout << "while r" << endl;
         char *recvBuf = (char *)&paket;
//        if (abs(lastDataReceived - lastDataSent) <= 10)
//        {
         if (recvfrom(server, recvBuf, sizeof(paket), 0, (struct sockaddr *)&serverAddr, &client_slen) >= 0)
         {
             cout << "--------------------------------" << endl;
             cout << "data rec" << endl;
//             cout << *lastDataReceived << endl;
             ((UdpConnectionInfo *) void_args)->lastDataReceived += 1;
             client_buffer->push_back(recvBuf);
         }
//        }
     }
 }

 void* sentData(void* void_args) {
     cout << "send Data" << endl;

     int server = ((UdpConnectionInfo *) void_args)->server;
     int client = ((UdpConnectionInfo *) void_args)->client;
     unsigned int client_slen = ((UdpConnectionInfo *) void_args)->client_slen;
     sockaddr_in *clientAddress = ((UdpConnectionInfo *) void_args)->clientAddress;
     sockaddr_in *clientAddr = ((UdpConnectionInfo *) void_args)->clientAddr;
     unsigned int server_slen = ((UdpConnectionInfo *) void_args)->server_slen;
     vector<char*> *buffer = ((UdpConnectionInfo *) void_args)->buffer;
     vector<char*> *client_buffer = ((UdpConnectionInfo *) void_args)->client_buffer;

     while(true) {
         if (!client_buffer->empty())
         {
             char *segment = client_buffer->at(reinterpret_cast<unsigned long>(((UdpConnectionInfo *) void_args)->lastDataSent));
             if (sendto(client, segment, sizeof(paket), 0, (struct sockaddr *)&clientAddress, client_slen) != -1)
             {
                 cout << "s data" << endl;
                 ((UdpConnectionInfo *) void_args)->lastDataSent += 1;
             }
         }
     }
 }

void* sentAck(void* void_args) {
    cout << "send Ack" << endl;

    int server = ((UdpConnectionInfo *) void_args)->server;
    int client = ((UdpConnectionInfo *) void_args)->client;
    unsigned int client_slen = ((UdpConnectionInfo *) void_args)->client_slen;
    sockaddr_in *clientAddress = ((UdpConnectionInfo *) void_args)->clientAddress;
    sockaddr_in *clientAddr = ((UdpConnectionInfo *) void_args)->clientAddr;
    sockaddr_in *serverAddr = ((UdpConnectionInfo *) void_args)->serverAddr;
    unsigned int server_slen = ((UdpConnectionInfo *) void_args)->server_slen;
    vector<char*> *buffer = ((UdpConnectionInfo *) void_args)->buffer;
    vector<char*> *client_buffer = ((UdpConnectionInfo *) void_args)->client_buffer;

    while(true) {
        if (!buffer->empty())
        {
            char *sendBuf = buffer->at(reinterpret_cast<unsigned long>(((UdpConnectionInfo *) void_args)->lastAckSent));
            if (sendto(server, sendBuf, sizeof(ack), 0, (struct sockaddr *)&serverAddr, server_slen) != -1)
            {
                cout << "sand ack" << endl;
                ((UdpConnectionInfo *) void_args)->lastAckSent += 1;
            }
        }
    }
}

void* receiveAck(void * void_args) {
    cout << "recieve  ack" << endl;
//     UdpConnectionInfo *args = (UdpConnectionInfo*)void_args;

    int server = ((UdpConnectionInfo *) void_args)->server;
    int client = ((UdpConnectionInfo *) void_args)->client;
    sockaddr_in *clientAddress = ((UdpConnectionInfo *) void_args)->clientAddress;
    sockaddr_in *clientAddr = ((UdpConnectionInfo *) void_args)->clientAddr;
    sockaddr_in *serverAddr = ((UdpConnectionInfo *) void_args)->serverAddr;
    unsigned int client_slen = sizeof(*serverAddr);
    unsigned int server_slen = ((UdpConnectionInfo *) void_args)->server_slen;
    vector<char*> *buffer = ((UdpConnectionInfo *) void_args)->buffer;
    vector<char*> *client_buffer = ((UdpConnectionInfo *) void_args)->client_buffer;
//     int *lastDataReceived = ((UdpConnectionInfo *) void_args)->lastDataReceived;

    cout << "vay" <<endl;
    srand(time(nullptr));
    while(true){
        cout << "while r" << endl;
        char *acksegment = (char *)&ack;
        if (recvfrom(client, acksegment, sizeof(ack), 0, (struct sockaddr *)&clientAddress, &server_slen) >= 0)
        {
            cout << "r ack" << endl;
            ((UdpConnectionInfo *) void_args)->lastAckReceived += 1;
            buffer->push_back(acksegment);
        }
    }
}

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

    pthread_t receive_data_t;
    pthread_t sent_data_t;
    pthread_t send_ack_t;
    pthread_t receive_ack_t;

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
    pthread_create(&receive_data_t, nullptr, receiveData, &client_t);
    pthread_create(&sent_data_t, nullptr, sentData, (void *)&client_t);
    pthread_create(&receive_ack_t, nullptr, receiveAck, &client_t);
    pthread_create(&send_ack_t, nullptr, sentAck, (void *)&client_t);
    while(true) {
        sleep(100);
    }
    return 0;
}