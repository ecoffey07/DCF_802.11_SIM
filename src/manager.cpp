


#include "manager.hpp"

#include <chrono>
#include <thread>
#include <math.h>

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
  senderA = new Sender(simParams.DIFS_duration, simParams.SIFS_duration, simParams.ack_rts_cts_size, dataSlots, simParams.CWo, simParams.CWmax);
  senderB = new Sender(simParams.DIFS_duration, simParams.SIFS_duration, simParams.ack_rts_cts_size, dataSlots, simParams.CWo, simParams.CWmax);

}

void Manager::start() {
  
  while (true) {
    if (totalSlots >= maxSlots) {
      break;
    }
    senderA->Tick();
    senderB->Tick(); 

    totalSlots += 1;
    totalSimTime += simParams.slot_duration * pow(10, -6);
    std::cout << "Slot #: " << totalSlots << " SenderA Status: " << convert(senderA->getState(), true) << std::endl;
    std::this_thread::sleep_for(std::chrono::microseconds(TICK_TIME));
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