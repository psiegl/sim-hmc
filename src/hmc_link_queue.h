#ifndef _HMC_LINK_QUEUE_H_
#define _HMC_LINK_QUEUE_H_

#include <cstdint>
#include <list>
#include <tuple>
#include "config.h"
#include "hmc_macros.h"

class hmc_link;
class hmc_link_fifo;
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
  unsigned bitrate;

#ifdef HMC_LOGGING
  hmc_link *link;
#endif /* #ifdef HMC_LOGGING */
#ifdef HMC_USES_NOTIFY
  hmc_notify *notify;
#endif /* #ifdef HMC_USES_NOTIFY */
  std::list< std::tuple<char*, unsigned, unsigned, uint64_t> > list;
  hmc_link_fifo *buf;


public:
  hmc_link_queue(uint64_t* cur_cycle, hmc_link_fifo *buf, hmc_notify *notify,
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
