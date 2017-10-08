#ifndef _HMC_SLID_H_
#define _HMC_SLID_H_

#include "hmc_module.h"

// dummy class which provides the id (as it is a module), if logging is turned on for the SLID ...

class hmc_slid : public hmc_module {
private:
  unsigned id;

public:
  hmc_slid(unsigned id) :
    id(id)
  {}
  ~hmc_slid(void) {}

  bool set_link(unsigned linkId, hmc_link* link, enum hmc_link_type linkType) {
    return true;
  }
  unsigned get_id(void) { return this->id; }
};

#endif /* #ifndef _HMC_SLID_H_ */
