


#include "sender.hpp"


Sender::Sender(std::string ID, int DIFSSlots, int SIFSSlots, int ackSlots, int dataSlots, int cwMin, int cwMax) {
  currentCSState = SenderCSStates::DIFS;
  currentState = SenderStates::DIFS;

  this->ID = ID;

  // We subtract one because I'm too lazy to fix the code
  this->DIFSSlots = DIFSSlots - 1;
  this->SIFSSlots = SIFSSlots - 1;
  this->ackSlots = ackSlots - 1;
  this->dataSlots = dataSlots - 1;

  DIFSCount = this->DIFSSlots;
  SIFSCount = this->SIFSSlots;
  ackCount = this->ackSlots;
  dataCount = this->dataSlots;

  contentionWindowMinSize = cwMin;
  contentionWindowMaxSize = cwMax;
  contentionWindowSize = cwMin;

  mediumBusy = false;

  std::uniform_int_distribution<int> distribution(0, contentionWindowSize - 1);
  std::random_device rd;
  std::default_random_engine generator(rd());
  backOffCount = distribution(generator); 
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
      mediumBusy = false;
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
      dataCount = dataSlots;
      currentState = SenderStates::SIFS;
    }
    else {
      dataCount--;
    }
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
        --framesInBuffer;
        contentionWindowSize = contentionWindowMinSize;

        std::uniform_int_distribution<int> distribution(0, contentionWindowSize - 1);
        std::random_device rd;
        std::default_random_engine generator(rd());
        backOffCount = distribution(generator);
        currentState = SenderStates::DIFS;
      }
      else {
        // No ack, double contention window, go back to DIFS
        contentionWindowSize *= 2;
        if (contentionWindowSize > contentionWindowMaxSize) {
          contentionWindowSize = contentionWindowMaxSize;
        }
        std::uniform_int_distribution<int> distribution(0, contentionWindowSize - 1);
        std::random_device rd;
        std::default_random_engine generator(rd());
        backOffCount = distribution(generator);
        currentState = SenderStates::DIFS;
      }
      ackCount = ackSlots;
    }
    else {
      --ackCount;
    }
    break;

  case SenderStates::SIFS:
    // Wait SIFS amount of slots, move to WAIT_FOR_ACK
    if (SIFSCount == 0) {
      SIFSCount = SIFSSlots;
      currentState = SenderStates::WAIT_FOR_ACK;
    }
    else {
      SIFSCount--;
    }
    break;

  case SenderStates::DIFS:
    // Wait DIFS amount of slots, move to Contention if something to send
    if (DIFSCount == 0) {
      DIFSCount = DIFSSlots;
      
      if (framesInBuffer > 0) {
        currentState = SenderStates::CONTENTION;
      }
      std::cout << "DIFS SYNC" << std::endl;
    }
    else {
      DIFSCount--;
    }
    if (mediumBusy) {
      dataCount = dataSlots + 1; // +1 to fix off by one error
      mediumBusy = false;
      currentState = SenderStates::DEFER;
    }
    break;

  case SenderStates::DEFER:
    // Wait defer length, then wait for sifs, then wait ack length
    if (dataCount == 0) {
      if (SIFSCount == 0) {
        if (ackCount == 0) {
          dataCount = dataSlots;
          SIFSCount = SIFSSlots;
          ackCount = ackSlots;
          mediumBusy = false;
          DIFSCount = DIFSSlots;
          currentState = SenderStates::DIFS;
        }
        else {
          ackCount--;
        }
      }
      else {
        SIFSCount--;
      }
    }
    else {
      dataCount--;
    }

    break;

  default:
    break;
  }
}

void Sender::setAck(bool ack) {
  ackReceived = ack;
}

void Sender::sendFrameToBuffer() {
  framesInBuffer++;
}

int Sender::getState() {
  return (int)currentState;
}

void Sender::setMediumBusy(bool busy) {
  mediumBusy = busy;
}
