
#pragma once

#include <iostream>
#include <random>

class Sender;

enum class SenderCSStates {
  CONTENTION,
  SEND = 1,
  REQUEST_TO_SEND,
  WAIT_FOR_ACK,
  SIFS,
  DIFS,
  DEFER
};

enum class SenderStates {
  CONTENTION,
  SEND = 1,
  WAIT_FOR_ACK,
  SIFS,
  DIFS,
  DEFER
};

class Sender {

  public:
    Sender(std::string ID, int DIFSSlots, int SIFSSlots, int ackSlots, int dataSlots, int cwMin, int cwMax);

    /**
     * Use Carrier Sensing
    */
    void TickWithCS();
    void Tick();

    void setMediumBusy(bool busy);
    void setAck(bool ack);
    void sendFrameToBuffer();

    int getState();

    
  private:
    
    SenderCSStates currentCSState;
    SenderStates currentState;

    std::string ID;

    bool mediumBusy;
    bool ackReceived;

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

    int dataSlots;
    int dataCount;


};