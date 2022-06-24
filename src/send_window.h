#ifndef SEND_WINDOW_H
#define SEND_WINDOW_H

typedef struct {
  unsigned int sendingWindowSize;
  unsigned int lastAckReceived;
  unsigned int lastFrameSent;
} SendingWindow;

#define NONE 0

#define SWS(S) (S).sendingWindowSize
#define LAR(S) (S).lastAckReceived
#define LFS(S) (S).lastFrameSent

void createNew(SendingWindow *s, unsigned int windowSize) {
  s->sendingWindowSize = windowSize;
  s->lastAckReceived = NONE;
  s->lastFrameSent = NONE;
}

void createNewSendingWindow(SendingWindow *s, unsigned int windowSize, unsigned int lastAckReceived, unsigned int lastFrameSent) {
  SWS(*s) = windowSize;
  LAR(*s) = lastAckReceived;
  LFS(*s) = lastFrameSent;
}

void receiveAcks(SendingWindow *s, int ack) {
  LAR(*s) = (LAR(*s) < ack) ? ack : LAR(*s);
}

void sendFrames(SendingWindow s, int frameCount) {
  // Valiate frameCount < SWS(s)
  frameCount = (frameCount < SWS(s)) ? frameCount : SWS(s);

  // Validate LFS - LAR <= SWS
  if (LFS(s) + frameCount > SWS(s) + LAR(s)) {
    // Sending too many frames, reduce count
    frameCount = SWS(s) + LAR(s) - LFS(s); 
  }

  LFS(s) += frameCount;
}

#endif