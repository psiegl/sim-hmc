#ifndef _HMC_CONN_RING_H_
#define _HMC_CONN_RING_H_

#include "hmc_connection.h"

class hmc_notify;
class hmc_cube;

class hmc_conn_ring : public hmc_connection {
private:
  unsigned routing(unsigned nextquad)
  {
    // since this is a ring, we can't cross from 0 to 3 or 1 to 2.
    // we will route first up then right
    /*
       [00]  <- ^= 0b1 -> [01]

       ^=0b10             ^=0b10

       [10]  <- ^= 0b1 -> [11]

       scheme routes first among x-axis, than y-axis
     */
    unsigned shift = (nextquad ^ this->id) & 0b01;
    return this->id ^ (0b10 >> shift);
  }
public:
  hmc_conn_ring(unsigned id, hmc_notify *notify, hmc_cube *cub) :
    hmc_connection(id, notify, cub)
  {}
  ~hmc_conn_ring(void)
  {}
};

#endif /* #ifndef _HMC_CONN_RING_H_ */
