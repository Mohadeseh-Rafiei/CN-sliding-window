#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include "utility.h"
#include "send_window.h"
#include <unistd.h>
#include <iostream>

using namespace std;

void die(char* errorMessage) {
	perror(errorMessage);
	exit(0);
}

int main(int argc, char* argv[]) {
	char* fileName = "./data/test.txt";
	unsigned int windowSize = 8;
	unsigned int bufferSize = 256;
	int port = 8000;

	int udpSocket;
	struct sockaddr_in clientAddress;

    unsigned char buffer[bufferSize];
    FILE* file = fopen(fileName, "r");
    if (file == NULL) {
        cout<<"file === null"<<endl;
        exit(0);
    }

    if((udpSocket = socket(AF_INET , SOCK_DGRAM , 0)) == 0)
    {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }
    memset((char *)&clientAddress, 0, sizeof(clientAddress));

    clientAddress.sin_family = AF_INET;
    clientAddress.sin_port = htons(port);
    clientAddress.sin_addr.s_addr = INADDR_ANY;

    struct timeval timeout;
    timeout.tv_sec = 0;
    timeout.tv_usec = 255000;

    if (setsockopt(udpSocket, SOL_SOCKET, SO_RCVTIMEO, (char *) &timeout, sizeof(timeout)) < 0) {
        perror("setsockopt");
        exit(EXIT_FAILURE);

    }


    unsigned int slen = sizeof(clientAddress);
    unsigned int i = 0;

    Segment paket;
    PacketACK ack;
    unsigned int counterBuffer = 0;
    unsigned int counterSequence = 0;
    int alreadyReadAll = 0;
    unsigned int advertisedWindowSize = 256;
    unsigned int bufferSizeOffset = 0;

    unsigned char c;

    SendingWindow sendingWindow;
    createNew(&sendingWindow, windowSize);

    unsigned int repeatEnd, repeatStart;
    unsigned int paramSend = 0;
    unsigned int iterator = 0;

    srand(time(NULL));
    while (1) {
        cout << "main while" << endl;
		while (counterBuffer < bufferSize && !alreadyReadAll) {
            if (fscanf(file, "%c", &c) == EOF) {
                alreadyReadAll = 1;
                break;
            }
            buffer[counterBuffer] = c;
            counterBuffer++;
        }
        repeatStart = LAR(sendingWindow);
        repeatEnd = LFS(sendingWindow);

        paramSend = 0;
        while (LFS(sendingWindow) < LAR(sendingWindow) + windowSize && LFS(sendingWindow) < counterBuffer + bufferSizeOffset && paramSend < advertisedWindowSize) {
            cout << "sent while" << endl;
            cout << buffer << endl;
            paket = CreateSegment(LFS(sendingWindow), buffer[LFS(sendingWindow) - bufferSizeOffset], 0);
            Checksum(paket) = generateChecksumPaket(paket);
            LFS(sendingWindow) = LFS(sendingWindow) + 1;

            char* segment = (char *) &paket;
            if (sendto(udpSocket, segment, sizeof(paket), 0, (struct sockaddr *) &clientAddress, slen) == -1) {
                die("sendto");
            }
            cout << "data sen" <<endl;
            paramSend++;
        }

        int i;
		for (i = repeatStart; i < repeatEnd; i++) {
			paket = CreateSegment(i, buffer[i - bufferSizeOffset], 0);
			Checksum(paket) = generateChecksumPaket(paket);

			char* segment = (char *) &paket;
			if (sendto(udpSocket, segment, sizeof(paket), 0, (struct sockaddr *) &clientAddress, slen) == -1) {
				die("sendto");
			}
            cout << "lost data sen" <<endl;
			paramSend++;
		}

		for (i = 0; i < paramSend; i++) {
			char* acksegment = (char *) &ack;
			if (recvfrom(udpSocket, acksegment, sizeof(ack), 0, (struct sockaddr*) &clientAddress, &slen) >= 0) {
				cout << "ack re" << endl;
                if (Checksum(ack) == generateChecksumACK(ack)) {
					advertisedWindowSize = AdvertisedWindowSize(ack);
					LAR(sendingWindow) = NextSequenceNumber(ack);
				} 
			}
		}
		

		if (alreadyReadAll == 1 && LAR(sendingWindow) > counterBuffer + bufferSizeOffset - 1) {
			break;
		}

		if (LAR(sendingWindow) == bufferSize + bufferSizeOffset) {
			counterBuffer = 0;
			bufferSizeOffset += bufferSize;
		}
	}

	PacketACK finalACK;
	NextSequenceNumber(finalACK) = 0;
	Segment finalSegment;
	finalSegment = CreateSegment(0, 0, 0);
	SOH(finalSegment) = 0x2;
	Checksum(finalSegment) = generateChecksumPaket(finalSegment);
	cout <<(generateChecksumPaket(finalSegment) == Checksum(finalSegment)) << endl;
	cout << NextSequenceNumber(finalACK) << endl;
	while (NextSequenceNumber(finalACK) == 0 || generateChecksumACK(finalACK) != Checksum(finalACK)) {
		char* segment = (char *) &finalSegment;
		sendto(udpSocket, segment, sizeof(finalSegment), 0, (struct sockaddr *) &clientAddress, slen);

		char* acksegment = (char *) &finalACK;
		recvfrom(udpSocket, acksegment, sizeof(finalACK), 0, (struct sockaddr*) &clientAddress, &slen);
	}

	printf("All data has been sent successfully\n");

	close(udpSocket);
}