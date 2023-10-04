

#include <iostream>
#include "manager.hpp"

int main() {
  

  Params params;
  params.frame_size = 1500;
  params.slot_duration = 10;
  params.SIFS_duration = 1;
  params.CWo = 8;
  params.lambda = 100;
  params.ack_rts_cts_size=3;
  params.DIFS_duration = 4;
  params.bandwidth = 10;
  params.CWmax = 512;
  params.sim_time = 10;

  Manager manager = Manager(params);
  manager.start();
}