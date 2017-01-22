#ifndef _HMC_MODULE_H_
#define _HMC_MODULE_H_

/**
 * Interface / base-class of each hmc module.
 */

#include "hmc_link.h"

class hmc_module {
public:
  hmc_module(void) {}
  virtual ~hmc_module(void) {}
  virtual bool set_link(unsigned linkId, hmc_link* link, enum hmc_link_type linkType) = 0;
  virtual unsigned get_id(void) = 0;
};

#endif /* #ifndef _HMC_MODULE_H_ */
