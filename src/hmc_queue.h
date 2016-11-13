#ifndef _HMC_QUEUE_H_
#define _HMC_QUEUE_H_

#include <list>
#include <cstdint>
#include <tuple>
#include "hmc_notify.h"

// tuple( packetptr, amount of cycles, totalsizeinbits );
class hmc_queue {
private:
  unsigned id;
  hmc_notify *notify;

  unsigned bitoccupation;
  unsigned bitoccupationmax;
  unsigned bitwidth;

  std::list< std::tuple<void*, unsigned, unsigned> > list;

public:
  hmc_queue(void);
  ~hmc_queue(void);

  void set_notify(unsigned id, hmc_notify *notify);
  void re_adjust(unsigned bitwidth, unsigned queuedepth);

  bool has_space(unsigned packetleninbit);
  int push_back(void *packet, unsigned packetleninbit);
  void* front(unsigned *packetleninbit);
  void* pop_front(void);
};

#endif /* #ifndef _HMC_QUEUE_H_ */
