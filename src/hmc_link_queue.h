#ifndef _HMC_LINK_QUEUE_H_
#define _HMC_LINK_QUEUE_H_

#include <list>
#include <stdint.h>
#include <tuple>
#include "config.h"
#include "hmc_macros.h"

class hmc_link;
class hmc_link_buf;
class hmc_notify;
class hmc_module;

// tuple( packetptr, amount of cycles, totalsizeinbits );
class hmc_link_queue {
private:
  unsigned id;
  unsigned notifyid;
  uint64_t *cur_cycle;

  unsigned bitoccupation;
  unsigned bitoccupationmax;

  unsigned bitwidth;
  float bitrate;

  hmc_notify *notify;
  std::list< std::tuple<char*, float, unsigned, uint64_t> > list;
  hmc_link_buf *buf;

  hmc_link *link;

public:
  hmc_link_queue(uint64_t* cur_cycle, hmc_link_buf *buf, hmc_notify *notify,
                 hmc_link *link);
  ~hmc_link_queue(void);

  void set_notifyid(unsigned notifyid, unsigned id);
  ALWAYS_INLINE unsigned get_id(void)
  {
    return this->id;
  }

  void re_adjust(unsigned link_bitwidth, float link_bitrate);

  bool has_space(unsigned packetleninbit);
  bool push_back(char *packet, unsigned packetleninbit);

  void clock(void);
};

#endif /* #ifndef _HMC_LINK_QUEUE_H_ */
