

#include <iostream>

#include "receiver.hpp"
#include "sender.hpp"

class Manager {
  public:
    Manager();
  private:
    Receiver receiver;
    Sender sender;
};