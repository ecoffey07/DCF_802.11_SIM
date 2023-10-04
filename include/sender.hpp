
#pragma once

#include <iostream>
#include <random>

class Sender;

enum class SenderCSStates {
  CONTENTION,
  REQUEST_TO_SEND,
  SEND,
  WAIT_FOR_ACK,
  SIFS,
  DIFS,
  DEFER
};

enum class SenderStates {
  CONTENTION,
  SEND,
  WAIT_FOR_ACK,
  SIFS,
  DIFS,
  DEFER
};

class Sender {

  public:
    Sender(int DIFSSlots, int SIFSSlots, int ackSlots, int dataSlots, int cwMin, int cwMax);

    /**
     * Use Carrier Sensing
    */
    void TickWithCS();

    void Tick();
    void TrigAck();

    int getState();

  private:
    
    SenderCSStates currentCSState;
    SenderStates currentState;

    std::default_random_engine engine;

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