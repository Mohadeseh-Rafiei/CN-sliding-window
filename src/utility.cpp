#include "utility.h"

unsigned char generateChecksumACK(PacketACK paket) {
  unsigned char result = 0;
  result += ACK(paket);
  result += AdvertisedWindowSize(paket) & 0xFF;
  result += NextSequenceNumber(paket) & 0xFF;
  return result;
}
 
unsigned char generateChecksumPaket(Segment paket) {
  unsigned char result = 0;
  result += SOH(paket);
  result += SequenceNumber(paket) & 0xFF;
  result += STX(paket);
  result += Data(paket);
  result += ETX(paket);
  return result;
}

Segment CreateSegment(unsigned int inputSequenceNumber, unsigned char inputData, unsigned char inputChecksum) {
  Segment Segmnt;
  SOH(Segmnt) = DefaultSOH;
  SequenceNumber(Segmnt) = inputSequenceNumber;
  STX(Segmnt) = DefaultSTX;
  Data(Segmnt) = inputData;
  ETX(Segmnt) = DefaultETX;
  Checksum(Segmnt) = inputChecksum;

  return Segmnt;
}

PacketACK CreatePacketACK(unsigned int inputNextSequenceNumber, unsigned int inputAdvertisedWindowSize, unsigned char inputChecksum) {
  PacketACK Packet;
  ACK(Packet) = DefaultACK;
  NextSequenceNumber(Packet) = inputNextSequenceNumber;
  AdvertisedWindowSize(Packet) = inputAdvertisedWindowSize;
  Checksum(Packet) = inputChecksum;

  return Packet;
}