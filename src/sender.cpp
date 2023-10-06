


#include "sender.hpp"


Sender::Sender(std::string ID, int DIFSSlots, int SIFSSlots, int ackSlots, int dataSlots, int cwMin, int cwMax) {
  currentState = SenderStates::DIFS;

  this->ID = ID;

  // We subtract one because I'm too lazy to fix the code
  this->DIFSSlots = DIFSSlots - 1;
  this->SIFSSlots = SIFSSlots - 1;
  this->dataSlots = dataSlots - 1;
  this->ackSlots = ackSlots - 1;
  this->ctsSlots = ackSlots - 1;
  this->rtsSlots = ackSlots - 1;
  
  DIFSCount = this->DIFSSlots;
  SIFSCount = this->SIFSSlots;
  dataCount = this->dataSlots;
  ackCount = this->ackSlots;
  rtsCount = this->rtsSlots;

  contentionWindowMinSize = cwMin;
  contentionWindowMaxSize = cwMax;
  contentionWindowSize = cwMin;

  mediumBusy = false;
  vcsEnabled = false;
  readyForClearance = false;

  std::uniform_int_distribution<int> distribution(0, contentionWindowSize - 1);
  std::random_device rd;
  std::default_random_engine generator(rd());
  backOffCount = distribution(generator); 
}

void Sender::TickWithCS() {

  switch (currentState) {

  case SenderStates::CONTENTION:  
    // Countdown from backoff counter, if backoff counter = 0, move to REQUEST_TO_SEND next tick
    break;

  case SenderStates::RTS:
    // Send RTS, 
    break;

  case SenderStates::SEND:
    break;
 
  case SenderStates::WAIT_FOR_ACK:
    break;

  case SenderStates::SIFS:
    break;

  case SenderStates::DIFS:

    if (DIFSCount == 0) {
      // Repeat DIFS or Move to contention if framesInBuffer > 0

      DIFSCount = DIFSSlots;
    }


    break;

  case SenderStates::DEFER:
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
      if (vcsEnabled) {
        rtsCount = 1 + ctsSlots + 1 + 4;
      }
      dataCount = dataSlots + 1;
      currentState = SenderStates::DEFER;
    }
    
    if (backOffCount == 0 && !mediumBusy) {
      if (vcsEnabled) {
        rtsCount = rtsSlots + 1 + ctsSlots + 1;
        currentState = SenderStates::RTS;
      }
      else {
        currentState = SenderStates::SEND;
      }
    }
    else {
      backOffCount--;
    }
    break;

  case SenderStates::SEND:
    // Send data frame, one slot at a time, if remaining data = 0, move to SIFS
    // std::cout << ID << " " << dataCount << std::endl;
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
  case SenderStates::RTS:
    
    // ReadyForClearance only used in Topography B part 2
    if (rtsCount == 1) {
      std::cout << ID << " READY FOR CLEARANCE" << std::endl;
      readyForClearance = true;
    }
    if (rtsCount == 0) {
      readyForClearance = false;
      if (clearToSend) {
        clearToSend = false;
        currentState = SenderStates::SEND;
      }
      else {
        // Collision Occured!
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
    }
    else {
      --rtsCount;
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
      rtsCount = 0;
      if (vcsEnabled) {
        rtsCount = 1 + ctsSlots + 1 + 3;
      }
      currentState = SenderStates::DEFER;
    }
    break;

  case SenderStates::DEFER:
    // Wait defer length, then wait for sifs, then wait ack length
    // If carrier sensing enabled, wait SIFS x2 + CTS + the rest of it
    mediumBusy = false;
    
    // std::cout << ID << " " << rtsCount << " " << dataCount << " " << SIFSCount << " " << ackCount << std::endl;
    // rtsCount includes SIFS, CTS, SIFS
    if (rtsCount == 0) {
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
    }
    else {
      --rtsCount;
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

void Sender::setVCS(bool vcs) {
  vcsEnabled = vcs;
}

void Sender::setClearToSend(bool clear) {
  clearToSend = clear;
}

bool Sender::getReadyForClearance() {
  return readyForClearance;
}