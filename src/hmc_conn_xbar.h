#ifndef _HMC_CONN_XBAR_H_
#define _HMC_CONN_XBAR_H_

#include <cstdint>
#include <list>
#include "hmc_connection.h"

class hmc_cube;
class hmc_link;
class hmc_notify;

class hmc_xbar_part : public hmc_conn_part {
private:
  unsigned routing(unsigned nextquad)
  {
    // just ship it to the next quad ...
    return nextquad;
  }
public:
  hmc_xbar_part(unsigned id, hmc_notify *notify, hmc_cube *cub) :
    hmc_conn_part(id, notify, cub)
  {}
  ~hmc_xbar_part(void)
  {}
};

class hmc_xbar : public hmc_conn {
public:
  hmc_xbar(hmc_notify *notify, hmc_cube *cub,
           unsigned ringbus_bitwidth, float ringbus_bitrate,
           uint64_t *clk);

  ~hmc_xbar(void);
};

#endif /* #ifndef _HMC_CONN_RING_H_ */

