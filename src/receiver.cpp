#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include "utility.h"
#include <fstream>
#include <iostream>
#include <math.h>
#include <unistd.h>

using namespace std;

unsigned int bufferSize;

void die(char *s) {
	perror(s);
	exit(1);
}

void flush(bool *alreadyReceived) {
	int i;
	for (i = 0; i < bufferSize; i++) {
		alreadyReceived[i] = false;
	}
}

int main(int argc, char* argv[]) {
	char* fileName = "./data/output.txt";
	unsigned int windowSize = 8;
	bufferSize = 256;
	int port = 8001;
	unsigned char buffer[bufferSize];
	bool alreadyReceived[bufferSize];

	int udpSocket;
	struct sockaddr_in serverAddr, clientAddr;
	unsigned int slen = sizeof(serverAddr); 

	if ((udpSocket = socket(AF_INET, SOCK_DGRAM, 0)) == -1) {
		 die("socket");
	}

	//Urusan Timeout
	struct timeval timeout;
	timeout.tv_sec = 0;
	timeout.tv_usec = 255000;
	setsockopt(udpSocket, SOL_SOCKET, SO_RCVTIMEO, (char *) &timeout, sizeof(timeout));
	

	clientAddr.sin_family = AF_INET;
	clientAddr.sin_port = htons(port);
	clientAddr.sin_addr.s_addr = htonl(INADDR_ANY);

	if (bind (udpSocket, (struct sockaddr *) &clientAddr, sizeof(clientAddr)) == -1) {
        die("bind");
    }

	FILE* file = fopen(fileName, "w");

	int recv_len;
	Segment paket;
	PacketACK ack;
	unsigned int lastFrameCorrect = 0;
	unsigned int LFR = 0;
	bool first = true;
	unsigned int LAF = windowSize;
	unsigned int counterBuffer = 0;
	unsigned int counterBufferOffset = 0;
	unsigned int advertisedWindowSize = (bufferSize > windowSize) ? windowSize : bufferSize;
	int finish = 0;

	flush(alreadyReceived);
	
	srand(time(NULL));
	while (1) {
		fflush(stdout);
		char * recvBuf = (char *) &paket;
		if (recvfrom(udpSocket, recvBuf, sizeof(paket), 0, (struct sockaddr *) &serverAddr, &slen) >= 0) {
            cout<<"data re"<<endl;
            if (generateChecksumPaket(paket) == Checksum(paket)) {
				if (SOH(paket) == 0x02) {
					finish = 1;
				}
				if (SequenceNumber(paket) >= LFR && SequenceNumber(paket) <= LAF) {
					if (SequenceNumber(paket) == LFR) {
						LFR++;
						buffer[SequenceNumber(paket) - counterBufferOffset] = Data(paket);
						counterBuffer++;
						advertisedWindowSize--;
						if (advertisedWindowSize == 0) {
							advertisedWindowSize = (bufferSize > windowSize) ? windowSize : bufferSize;
						}
					}
				}

				LAF = LFR + min(windowSize, advertisedWindowSize);
			}
		}

		ack = CreatePacketACK(LFR, advertisedWindowSize, '0');
		Checksum(ack) = generateChecksumACK(ack);

		char *sendBuf = (char *) &ack;
		sendto(udpSocket, sendBuf, sizeof(ack), 0, (struct sockaddr*) &serverAddr, slen);
        cout<< "ack se" <<endl;
		if (counterBuffer == bufferSize) {
			counterBufferOffset += bufferSize;
			counterBuffer = 0;
			for (int i = 0; i < bufferSize; i++) {
				fputc(buffer[i], file);	
			}
		}

		if (finish) {
			break;
		}
	}
	if (counterBuffer != 0) {
		printf("Writing remaining buffer to File\n");
		for (int i = 0; i < counterBuffer; i++) {
			fputc(buffer[i], file);
		}
	}
	printf("All data has been received succesfully\n");
	
	fclose(file);
}