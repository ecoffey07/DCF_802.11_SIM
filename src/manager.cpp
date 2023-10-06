
#define TESTING
#define TOP1A
// #define TOP1B
// #define TOP2A
// #define TOP2B


#include "manager.hpp"
#include "matplotlibcpp.h"

#include <chrono>
#include <thread>
#include <math.h>
#include <cmath>
#include <random>
#include <queue>

/**
Quick Maths:
  Slot duration = 10us
  Data frame size = 1500 bytes
  BW = 10 Mbps
  
  Data Frame Slot Duration = Data frame size * 8 * 1E6 / BW / 1024 / 1024 / 10 = 114.4 = 115

*/


namespace plt = matplotlibcpp;

std::string convert(int state, bool cw); 

Manager::Manager(Params params) {
  simParams = params;
  maxSlots = simParams.sim_time / (simParams.slot_duration * pow(10, -6));
  int dataSlots = ceil(simParams.frame_size * 8 * pow(10,6) / simParams.bandwidth / 1024 / 1024 / 10);
  std::cout << "Data Slots: " << dataSlots << std::endl;

  //sender = Sender(simParams.DIFS_duration, simParams.SIFS_duration);
  // receiver = Receiver();
  senderA = new Sender("A", simParams.DIFS_duration, simParams.SIFS_duration, simParams.ack_rts_cts_size, dataSlots, simParams.CWo, simParams.CWmax);
  senderB = new Sender("B", simParams.DIFS_duration, simParams.SIFS_duration, simParams.ack_rts_cts_size, dataSlots, simParams.CWo, simParams.CWmax);

}

void Manager::start() {

  
  
  
  // Using algorithm given in Appendix 1
  std::uniform_int_distribution<int> distribution(1,99);
  std::random_device rd;
  std::default_random_engine generator(rd());
  std::vector<std::vector<int>> senderA_frames;
  std::vector<std::vector<int>> senderB_frames;

  std::vector<int> lambda_vector = {100,200,300,500,800,1000};
  
  // Generate Poisson Traffic
  for (unsigned int i = 0; i < lambda_vector.size(); i++) {
    std::vector<int> tempA;
    std::vector<int> tempB;
    int ya = 0;
    int yb = 0;
    int totalFrames = lambda_vector[i] * simParams.sim_time;
    for (int k = 0; k < totalFrames; k++) {
      double randNum = distribution(generator) / 100.0;
      double x = -1 * log(1 - randNum) / lambda_vector[i] / (simParams.slot_duration * pow(10, -6));
      ya += ceil(x);
      tempA.push_back(ya);
    }
    for (int k = 0; k < totalFrames; k++) {
      double randNum = distribution(generator) / 100.0;
      double x = -1 * log(1 - randNum) / lambda_vector[i] / (simParams.slot_duration * pow(10, -6));
      yb += ceil(x);
      tempB.push_back(ya);
    }
    senderA_frames.push_back(tempA);
    senderB_frames.push_back(tempB);
  }

  // End algorithm
  std::vector<int> testingA{10,15,20};
  std::vector<int> testingB{5,10,15,20};

#ifdef TESTING
  senderA_frames[0] = testingA;
  senderB_frames[0] = testingB;
#endif

//   int nextAFrame = senderA_frames.front();
//   senderA_frames.pop();
//   int nextBFrame = senderB_frames.front();
//   senderB_frames.pop();

  // Performance Metric Variables
  // int numCollisions = 0;
  // int sentFramesA = 0;
  // int sentFramesB = 0;
  // double throughputA = 0; // Throughput = data amount transfered / time
  // double throughputB = 0;


  // int receiveFrameCount = 0;
  bool collision = false;

  // Run Topology 1, no carrier sensing
#ifdef TOP1A
  double totalSimTime = 0.0;
  int totalSlots = 0;
  senderA->setVCS(false);
  senderB->setVCS(false);
  for (unsigned int i = 0; i < lambda_vector.size(); ++i) {
    totalSimTime = 0.0;
    totalSlots = 0;
    int AFrameCount = 0;
    int BFrameCount = 0;
    int nextAFrame;
    int nextBFrame;
    nextAFrame = senderA_frames[i][AFrameCount];
    nextBFrame = senderB_frames[i][BFrameCount];
    AFrameCount++;
    BFrameCount++;
    while (true) {
      // Break if end of Sim Time
      if (totalSlots >= maxSlots) {
        break;
      }
      // Check if Sender has something to send
      if (nextAFrame == totalSlots) {
        std::cout << "Sending Frame to Sender A Buffer" << std::endl;
        nextAFrame = senderA_frames[i][AFrameCount];
        AFrameCount++;
        senderA->sendFrameToBuffer();
      }
      if (nextBFrame == totalSlots) {
        std::cout << "Sending Frame to Sender B Buffer" << std::endl;
        nextBFrame = senderB_frames[i][BFrameCount];
        BFrameCount++;
        senderB->sendFrameToBuffer();
      }

      if (senderA->getState() == 1 && senderB->getState() == 1) {
        // Collision! Both senders are sending! Don't ACK
        std::cout << "Collision!" << std::endl;
        collision = true;
        senderA->setAck(false);
        senderB->setAck(false);
      }
      else if (senderA->getState() == 1 && senderB->getState() != 1) {
        // No Collision, senderA sending
        senderA->setAck(true);
        senderB->setMediumBusy(true);
      }
      else if (senderA->getState() != 1 && senderB->getState() == 1) {
        // No Collision, senderB sending
        senderB->setAck(true);
        senderA->setMediumBusy(true);
      }

      if (senderA->getState() != 1 && senderB->getState() != 1 && collision) {
        // Collision happened, reset
        collision = false;
      }

      senderA->Tick();
      senderB->Tick(); 

      totalSlots += 1;
      totalSimTime += simParams.slot_duration * pow(10, -6);
      std::cout << "TOP1A Slot #: " << totalSlots << " SenderA Status: " << convert(senderA->getState(), true) << " | SenderB Status: " << convert(senderB->getState(), true) << std::endl;
      // std::this_thread::sleep_for(std::chrono::microseconds(TICK_TIME));
      getchar();
    }
#ifdef TESTING
      // getchar();
#endif
    std::cout << i << std::endl;
  }
#endif

  // getchar();
  // Run Topology 1 with carrier sensing
#ifdef TOP1B
  collision = false;
  std::cout << "CORE" << std::endl;
  senderA_frames = A_frames;
  std::cout << "CORE" << std::endl;
  senderB_frames = B_frames;
  std::cout << "CORE" << std::endl;
  senderA->setVCS(true);
  senderB->setVCS(true);
  while (true) {

    // Break if end of Sim Time
    if (totalSlots >= maxSlots) {
      break;
    }

    // Check if Sender has something to send
    if (nextAFrame == totalSlots) {
      std::cout << "Sending Frame to Sender A Buffer" << std::endl;
      nextAFrame = senderA_frames.front();
      senderA_frames.pop();
      senderA->sendFrameToBuffer();
    }
    if (nextBFrame == totalSlots) {
      std::cout << "Sending Frame to Sender B Buffer" << std::endl;
      nextBFrame = senderB_frames.front();
      senderB_frames.pop();
      senderB->sendFrameToBuffer();
    }

    // if (senderA->getState() == 1 && senderB->getState() == 1) {
    //   // Collision! Both senders are sending! Don't ACK
    //   std::cout << "Collision!" << std::endl;
    //   collision = true;
    //   senderA->setAck(false);
    //   senderB->setAck(false);
    // }
    if (senderA->getState() == 6 && senderB->getState() == 6) {
      // Collision with VCS! Don't ACK
      std::cout << "Collision!" << std::endl;
      collision = true;
      senderA->setClearToSend(false);
      senderB->setClearToSend(false);
    }
    else if (senderA->getState() == 6 && senderB->getState() != 6) {
      // No Collision, senderA sending
      senderA->setClearToSend(true);
      senderB->setMediumBusy(true);
    }
    else if (senderB->getState() == 6 && senderA->getState() != 6) {
      // No Collision, senderB sending
      senderB->setClearToSend(true);
      senderA->setMediumBusy(true);
    }

    // ACK always if they are in the waiting for ack state, no collisions possible here due to VCS
    if (senderA->getState() == 2) {
      senderA->setAck(true);
    }
    if (senderB->getState() == 2) {
      senderB->setAck(true);
    }

    senderA->Tick();
    senderB->Tick(); 

    totalSlots += 1;
    totalSimTime += simParams.slot_duration * pow(10, -6);
    std::cout << "TOP1B Slot #: " << totalSlots << " SenderA Status: " << convert(senderA->getState(), true) << " | SenderB Status: " << convert(senderB->getState(), true) << std::endl;
    std::this_thread::sleep_for(std::chrono::microseconds(TICK_TIME));
#ifdef TESTING
    getchar();
#endif
  }
#endif

// getchar();
// Run Topology 2 no carrier sensing
#ifdef TOP2A
  collision = false;
  senderA_frames = A_frames;
  senderB_frames = B_frames;
  senderA->setVCS(false);
  senderB->setVCS(false);
  while (true) {

    // Break if end of Sim Time
    if (totalSlots >= maxSlots) {
      break;
    }

    // Check if Sender has something to send
    if (nextAFrame == totalSlots) {
      std::cout << "Sending Frame to Sender A Buffer" << std::endl;
      nextAFrame = senderA_frames.front();
      senderA_frames.pop();
      senderA->sendFrameToBuffer();
    }
    if (nextBFrame == totalSlots) {
      std::cout << "Sending Frame to Sender B Buffer" << std::endl;
      nextBFrame = senderB_frames.front();
      senderB_frames.pop();
      senderB->sendFrameToBuffer();
    }

    if (senderA->getState() == 1 && senderB->getState() == 1) {
      // Collision! Both senders are sending! Don't ACK
      std::cout << "Collision!" << std::endl;
      collision = true;
      senderA->setAck(false);
      senderB->setAck(false);
    }
    else if (senderA->getState() != 1 && senderB->getState() != 1 && collision) {
      // Both nodes, aren't sending, reset collision
      collision = false;
    }

    // If A is sending and no collision
    if (senderA->getState() == 1 && !collision) {
      senderA->setAck(true);
    }

    // If B is sending and no collision
    if (senderB->getState() == 1 && !collision) {
      senderB->setAck(true);
    }

    senderA->Tick();
    senderB->Tick(); 

    totalSlots += 1;
    totalSimTime += simParams.slot_duration * pow(10, -6);
    std::cout << "TOP2A Slot #: " << totalSlots << " SenderA Status: " << convert(senderA->getState(), true) << " | SenderB Status: " << convert(senderB->getState(), true) << std::endl;
    std::this_thread::sleep_for(std::chrono::microseconds(TICK_TIME));
#ifdef TESTING
    getchar();
#endif
  }
#endif
  // getchar();
  // Run Topology 2 with carrier sensing
#ifdef TOP2B
  collision = false;
  senderA_frames = A_frames;
  senderB_frames = B_frames;
  senderA->setVCS(true);
  senderB->setVCS(true);

  int requestToSendCount = 0;
  bool skipA;
  bool skipB;
  while (true) {

    // Break if end of Sim Time
    if (totalSlots >= maxSlots) {
      break;
    }

    // Check if Sender has something to send
    if (nextAFrame == totalSlots) {
      std::cout << "Sending Frame to Sender A Buffer" << std::endl;
      nextAFrame = senderA_frames.front();
      senderA_frames.pop();
      senderA->sendFrameToBuffer();
    }
    if (nextBFrame == totalSlots) {
      std::cout << "Sending Frame to Sender B Buffer" << std::endl;
      nextBFrame = senderB_frames.front();
      senderB_frames.pop();
      senderB->sendFrameToBuffer();
    }

    // if (senderA->getState() == 1 && senderB->getState() == 1) {
    //   // Collision! Both senders are sending! Don't ACK
    //   std::cout << "Collision!" << std::endl;
    //   collision = true;
    //   senderA->setAck(false);
    //   senderB->setAck(false);
    // }
    if (senderA->getState() == 1 || senderB->getState() == 1) {
      // Collision with VCS! Don't ACK
      senderA->setClearToSend(false);
      senderB->setClearToSend(false);
    }
    if (senderA->getState() == 6 && senderB->getState() == 6) {
      // Collision with VCS! Don't ACK
      std::cout << "Collision!" << std::endl;
      collision = true;
      senderA->setClearToSend(false);
      senderB->setClearToSend(false);
    }
    else if (senderA->getReadyForClearance() && senderB->getState() != 6 && !collision) {
      // No Collision, senderA sending
      senderA->setClearToSend(true);
      senderB->setMediumBusy(true);
    }
    else if (senderB->getReadyForClearance() && senderA->getState() != 6 && !collision) {
      // No Collision, senderB sending
      senderB->setClearToSend(true);
      senderA->setMediumBusy(true);
    }

    if (senderA->getState() != 6 && senderB->getState() != 6) {
      collision = false;
    }

    // At this point, I'm hardcoding values to get this done...
    // just make sure DIFS sync up after sending
    if (senderA->getState() == 2 && !skipB) {
      skipB = true;
      senderB->setdataCount(0);
      senderB->setrtsCount(0);
      senderB->setSIFSCount(0);
      senderB->setackCount(2);
    }
    if (senderA->getState() != 2) {
      skipB = false;
    }

    if (senderB->getState() == 2 && !skipA) {
      skipA = true;
      senderA->setdataCount(0);
      senderA->setrtsCount(0);
      senderA->setSIFSCount(0);
      senderA->setackCount(2);
    }
    if (senderB->getState() != 2) {
      skipA = false;
    }

    // ACK always if they are in the waiting for ack state, no collisions possible here due to VCS
    if (senderA->getState() == 2) {
      senderA->setAck(true);
    }
    if (senderB->getState() == 2) {
      senderB->setAck(true);
    }

    senderA->Tick();
    senderB->Tick(); 

    totalSlots += 1;
    totalSimTime += simParams.slot_duration * pow(10, -6);
    std::cout << "TOP2B Slot #: " << totalSlots << " SenderA Status: " << convert(senderA->getState(), true) << " | SenderB Status: " << convert(senderB->getState(), true) << std::endl;
    std::this_thread::sleep_for(std::chrono::microseconds(TICK_TIME));
#ifdef TESTING
    getchar();
#endif
  }
#endif

}


std::string convert(int state, bool cw) {
  
  if (cw) {
    switch(state) {
      case(0): return "CONTENTION";
      case(1): return "SEND";
      case(2): return "WAIT_FOR_ACK";
      case(3): return "SIFS";
      case(4): return "DIFS";
      case(5): return "DEFER";
      case(6): return "RTS";
      default: return "Broken";
    }
  }
  else {

  }
  return "ERROR";
}