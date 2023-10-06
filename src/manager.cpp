
//#define TESTING

#include "manager.hpp"

#include <chrono>
#include <thread>
#include <math.h>
#include <random>
#include <queue>

/**
Quick Maths:
  Slot duration = 10us
  Data frame size = 1500 bytes
  BW = 10 Mbps
  
  Data Frame Slot Duration = Data frame size * 8 * 1E6 / BW / 1024 / 1024 / 10 = 114.4 = 115

*/
std::string convert(int state, bool cw); 

Manager::Manager(Params params) {
  simParams = params;
  totalSimTime = 0.0;
  totalSlots = 0;
  maxSlots = simParams.sim_time / (simParams.slot_duration * pow(10, -6));
  int dataSlots = ceil(simParams.frame_size * 8 * pow(10,6) / simParams.bandwidth / 1024 / 1024 / 10);
  std::cout << "Data Slots: " << dataSlots << std::endl;

  //sender = Sender(simParams.DIFS_duration, simParams.SIFS_duration);
  // receiver = Receiver();
  senderA = new Sender("A", simParams.DIFS_duration, simParams.SIFS_duration, simParams.ack_rts_cts_size, dataSlots, simParams.CWo, simParams.CWmax);
  senderB = new Sender("B", simParams.DIFS_duration, simParams.SIFS_duration, simParams.ack_rts_cts_size, dataSlots, simParams.CWo, simParams.CWmax);

}

void Manager::start() {

  // Generate Poisson Traffic
  int totalFrames = simParams.lambda * simParams.sim_time;
  
  // Using algorithm given in Appendix 1
  std::default_random_engine gen; 
  std::uniform_int_distribution<int> distribution(1,99);
  gen.seed(500);
  std::queue<int> senderA_frames;
  std::queue<int> senderB_frames;
  int ya = 0;
  int yb = 0;
  for (int i = 0; i < totalFrames; ++i) {
    // generate random frames for Sender A
    double randNum = distribution(gen) / 100.0;
    double x = -1 * log(1 - randNum) / simParams.lambda / (simParams.slot_duration * pow(10, -6));
    ya += ceil(x);
    senderA_frames.push(ya);

    // generate random frames for Sender B
    randNum = distribution(gen) / 100.0;
    x = -1 * log(1 - randNum) / simParams.lambda / (simParams.slot_duration * pow(10, -6));
    yb += ceil(x);
    senderB_frames.push(yb);
  }
  // End algorithm
  std::queue<int> testingA({10,15,20});
  std::queue<int> testingB({5,10,15,20});


#ifdef TESTING
  senderA_frames = testingA;
  senderB_frames = testingB;
#endif

  int nextAFrame = senderA_frames.front();
  senderA_frames.pop();
  int nextBFrame = senderB_frames.front();
  senderB_frames.pop();

  // int receiveFrameCount = 0;
  // bool collision = false;

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
      senderA->setAck(false);
      senderB->setAck(false);
    }
    else if (senderA->getState() == 1 && senderB->getState() != 1) {
      // No Collision, senderA sending
      senderA->setAck(true);
      senderB->setMediumBusy(true);
    }
    else if (senderB->getState() == 1 && senderA->getState() != 1) {
      // No Collision, senderB sending
      senderB->setAck(true);
      senderA->setMediumBusy(true);
    }


    senderA->Tick();
    senderB->Tick(); 

    totalSlots += 1;
    totalSimTime += simParams.slot_duration * pow(10, -6);
    std::cout << "Slot #: " << totalSlots << " SenderA Status: " << convert(senderA->getState(), true) << " | SenderB Status: " << convert(senderB->getState(), true) << std::endl;
    std::this_thread::sleep_for(std::chrono::microseconds(TICK_TIME));
    //getchar();
  }
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
      default: return "Broken";
    }
  }
  else {

  }
  return "ERROR";
}