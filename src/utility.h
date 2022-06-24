#ifndef _UTILITY_H_
#define _UTILITY_H_


#include <string>
#include <stdint.h>

using namespace std;

typedef struct {
  unsigned char SOH;
  unsigned int SequenceNumber;
  unsigned char STX;
  unsigned char Data;
  unsigned char ETX;
  unsigned char Checksum;
} Segment;

typedef struct {
  unsigned char ACK;
  unsigned int NextSequenceNumber;
  unsigned int AdvertisedWindowSize;
  unsigned char Checksum;
} PacketACK;

#define DefaultSOH 0x1;
#define DefaultSTX 0x2;
#define DefaultETX 0x3;
#define DefaultACK 0x6;

#define SOH(S) ((S).SOH)
#define SequenceNumber(S) ((S).SequenceNumber)
#define STX(S) ((S).STX)
#define Data(S) ((S).Data)
#define ETX(S) ((S).ETX)
#define Checksum(S) ((S).Checksum)
#define ACK(P) ((P).ACK)
#define NextSequenceNumber(P) ((P).NextSequenceNumber)
#define AdvertisedWindowSize(P) ((P).AdvertisedWindowSize)


const string currentDateTime();

Segment CreateSegment(unsigned int inputSequenceNumber, unsigned char inputData, unsigned char inputChecksum);

PacketACK CreatePacketACK(unsigned int inputNextSequenceNumber, unsigned int inputAdvertisedWindowSize, unsigned char inputChecksum);

unsigned char generateChecksumPaket(Segment paket);

unsigned char generateChecksumACK(PacketACK paket);
#endif