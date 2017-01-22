#ifndef _HMC_CONN_XBAR_H_
#define _HMC_CONN_XBAR_H_

#include "hmc_connection.h"

class hmc_notify;
class hmc_cube;

class hmc_conn_xbar : public hmc_connection {
private:
  unsigned routing(unsigned nextquad)
  {
    // just ship it to the next quad ...
    return nextquad;
  }
public:
  hmc_conn_xbar(unsigned id, hmc_notify *notify, hmc_cube *cub) :
    hmc_connection(id, notify, cub)
  {}
  ~hmc_conn_xbar(void)
  {}
};

#endif /* #ifndef _HMC_CONN_RING_H_ */

