
#pragma once

#include <iostream>

#include "sender.hpp"

struct Params {
  int frame_size; // bytes
  int slot_duration; // microseconds
  int SIFS_duration; //slots
  int CWo; // contention window min size, slots
  int lambda; // frames per second
  int ack_rts_cts_size; // slots
  int DIFS_duration; // slots
  int bandwidth; // megabits per second
  int CWmax; // contention window max size, slots
  int sim_time; // seconds
};

const int TICK_TIME = 1000; // us between simulated ticks

class Manager {
  public:
    Manager(Params params);
    
    void start();

  private:
    Params simParams;
    Sender* senderA;
    Sender* senderB;

    double dataSlots;
    int maxSlots;

};