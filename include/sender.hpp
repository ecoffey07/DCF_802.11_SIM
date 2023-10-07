
#pragma once

#include <iostream>
#include <random>

class Sender;

enum class SenderStates {
  CONTENTION,
  SEND = 1,
  WAIT_FOR_ACK,
  SIFS,
  DIFS,
  DEFER,
  RTS
};

class Sender {

  public:
    Sender(std::string ID, int DIFSSlots, int SIFSSlots, int ackSlots, int dataSlots, int cwMin, int cwMax);

    void Tick();

    void setMediumBusy(bool busy);
    void setAck(bool ack);
    void setClearToSend(bool clear);
    void sendFrameToBuffer();
    void setVCS(bool vcs);
    void setState(int state);
    void reset();

    bool getReadyForClearance();
    bool frameSuccess() {return sentFrame;};
    int getState();

    void setdataCount(int dataCount) { this->dataCount = dataCount; };
    void setrtsCount(int rtsCount) { this->rtsCount = rtsCount;};
    void setSIFSCount(int SIFSCount) { this->SIFSCount = SIFSCount;};
    void setackCount(int ackCount) { this->ackCount = ackCount;};
    
  private:
    
    SenderStates currentState;

    std::string ID;

    bool mediumBusy;
    bool ackReceived;
    bool clearToSend;
    bool readyForClearance;
    bool sentFrame;

    bool vcsEnabled;

    int framesInBuffer;
    int backOffCount;

    int contentionWindowMaxSize;
    int contentionWindowMinSize;
    int contentionWindowSize;
    
    int DIFSSlots;
    int DIFSCount;
    
    int SIFSSlots;
    int SIFSCount;
    
    int ackSlots;
    int ackCount;

    int rtsSlots;
    int rtsCount;

    int ctsSlots;

    int dataSlots;
    int dataCount;


};