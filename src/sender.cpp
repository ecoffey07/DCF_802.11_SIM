


#include "sender.hpp"


Sender::Sender(int DIFSSlots, int SIFSSlots, int ackSlots, int dataSlots, int cwMin, int cwMax) {
  currentCSState = SenderCSStates::DIFS;
  currentState = SenderStates::DIFS;

  this->DIFSSlots = DIFSSlots;
  this->SIFSSlots = SIFSSlots;
  this->ackSlots = ackSlots;

  DIFSCount = DIFSSlots;
  SIFSCount = SIFSSlots;
  ackCount = ackSlots;

  cwMin = cwMin;
  cwMax = cwMax;
  contentionWindowSize = cwMin;
}

void Sender::TickWithCS() {

  switch (currentCSState) {

  case SenderCSStates::CONTENTION:  
    // Countdown from backoff counter, if backoff counter = 0, move to REQUEST_TO_SEND next tick
    break;

  case SenderCSStates::REQUEST_TO_SEND:
    // Send RTS, 
    break;

  case SenderCSStates::SEND:
    break;

  case SenderCSStates::WAIT_FOR_ACK:
    break;

  case SenderCSStates::SIFS:
    break;

  case SenderCSStates::DIFS:

    if (DIFSCount == 0) {
      // Repeat DIFS or Move to contention if framesInBuffer > 0

      DIFSCount = DIFSSlots;
    }


    break;

  case SenderCSStates::DEFER:
    break;

  default:
    break;
  }
}

void Sender::Tick() {
  
  switch (currentState) {

  case SenderStates::CONTENTION:
    // Countdown from backoff counter, if backoff counter = 0, move to SEND
    // If medium is busy, move to DEFER
    
    if (mediumBusy) {
      currentState = SenderStates::DEFER;
    }
    
    if (backOffCount == 0) {
      currentState = SenderStates::SEND;
    }
    else {
      backOffCount--;
    }
    break;

  case SenderStates::SEND:
    // Send data frame, one slot at a time, if remaining data = 0, move to SIFS
    if (dataCount == 0) {
      currentState = SenderStates::SIFS;
    }
    dataCount--;
    break;

  case SenderStates::WAIT_FOR_ACK:
    // Wait for acknowledgement from rceiver, move to DIFS after ACK
    // Reset CW size if Ack received
    // If no ACK received, double CW up to CWmax time
    // Also reset backoff counter here
    if (ackCount == 0) {
      if (ackReceived) {
        // Reset contention window max size, pick new backOff
        ackReceived = false;
      }
      else {
        // No ack, double contention window, go back to DIFS
      }
    }
    break;

  case SenderStates::SIFS:
    // Wait SIFS amount of slots, move to WAIT_FOR_ACK
    if (SIFSCount == 0) {
      currentState = SenderStates::WAIT_FOR_ACK;
    }
    SIFSCount--;
    break;

  case SenderStates::DIFS:
    // Wait DIFS amount of slots, move to Contention if something to send
    if (DIFSCount == 0) {
      DIFSCount = DIFSSlots;
      std::uniform_int_distribution<int> distrub(0, contentionWindowSize - 1);
      std::cout << distrub(engine) << std::endl;
      if (framesInBuffer > 0) {
        currentState = SenderStates::CONTENTION;
      }
      std::cout << "DIFS SYNC" << std::endl;
    }
    DIFSCount--;
    break;

  case SenderStates::DEFER:
    // Wait defer length, then wait for sifs, then wait ack length
    if (dataCount == 0) {
      if (SIFSCount == 0) {
        if (ackCount == 0) {
          dataCount = dataSlots;
          SIFSCount = SIFSSlots;
          ackCount = ackSlots;
          currentState = SenderStates::DIFS;
        }
        ackCount--;
      }
      SIFSCount--;
    }
    dataCount--;
    break;

  default:
    break;
  }
}

void Sender::TrigAck() {


}

int Sender::getState() {
  return (int)currentState;
}