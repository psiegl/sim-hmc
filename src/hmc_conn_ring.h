#ifndef _HMC_CONN_RING_H_
#define _HMC_CONN_RING_H_

#include <array>
#include <list>
#include "hmc_connection.h"
#include "hmc_notify.h"

class hmc_cube;
class hmc_link;

class hmc_ring_part : public hmc_connection {
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
  hmc_ring_part(unsigned id, hmc_notify *notify, hmc_cube *cub) :
    hmc_connection(id, notify, cub)
  {}
  ~hmc_ring_part(void)
  {}
};


class hmc_ring : public hmc_conn {
private:
  hmc_notify conn_notify;
  std::array<hmc_ring_part*, HMC_NUM_QUADS> conns;
  std::list<hmc_link*> link_garbage;

public:
  hmc_ring(hmc_notify *notify, hmc_cube *cub,
           unsigned ringbus_bitwidth, float ringbus_bitrate,
           uint64_t *clk);

  ~hmc_ring(void);

  hmc_ring_part* get_conn(unsigned id) {
    return this->conns[id];
  }

  unsigned get_id(void) { return 0; }

  void clock(void);
  bool notify_up(void);
};

#endif /* #ifndef _HMC_CONN_RING_H_ */
