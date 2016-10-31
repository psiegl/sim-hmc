#include <iostream>

class notify {

public:
  unsigned HMC_needs_clocking;

  notify(void) {};
  ~notify(void) {};

  void add(unsigned id) {
    this->HMC_needs_clocking |= (0x1 << id);
    std::cout << "I was called -> go to bed!" << std::endl;
  }
  void del(unsigned id) {
    this->HMC_needs_clocking &= (0x1 << id);
  }

};
