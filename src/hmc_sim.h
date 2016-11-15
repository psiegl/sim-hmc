#ifndef _HMC_SIM_H_
#define _HMC_SIM_H_

#include <cstdint>
#include <map>
#include <list>
#include "config.h"
#include "hmc_cube.h"
#include "hmc_notify.h"

class hmc_link;
class hmc_cube;

typedef enum{
  RD_RS     = 0x38,
  WR_RS     = 0x39,
  MD_RD_RS  = 0x3A,
  MD_WR_RS  = 0x3B,
  RSP_ERROR = 0x3E,
  RSP_NONE  = 0x00,	// not really defined, but let's take it
  RSP_CMC
} hmc_response_t;

class hmc_sim : private hmc_notify_cl {
private:
  uint64_t clk;
  hmc_notify cubes_notify;
  std::map<unsigned, hmc_cube*> cubes;

  std::map<unsigned, hmc_notify*> slidnotify;

  std::list<hmc_link*> link_garbage;

  bool notify_up(void);

public:
  hmc_sim(unsigned num_hmcs, unsigned num_slids,
          unsigned num_links, unsigned capacity,
          enum link_width_t ringbuswidth,
          enum link_width_t vaultbuswidth);
  ~hmc_sim(void);

  bool hmc_link_config(unsigned src_hmcId, unsigned src_linkId,
                       unsigned dst_hmcId, unsigned dst_linkId,
                       enum link_width_t bitwidth);
  hmc_link* hmc_link_to_slid(unsigned slidId, unsigned hmcId,
                             unsigned linkId, enum link_width_t bitwidth);

  void clock(void);
  ALWAYS_INLINE unsigned get_num_cubes(void)
  {
    return this->cubes.size();
  }
  ALWAYS_INLINE hmc_cube* get_cube(unsigned id)
  {
    return this->cubes[id];
  }
};

#endif /* #ifndef _HMC_SIM_H_ */
