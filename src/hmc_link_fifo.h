#ifndef _HMC_LINK_BUF_H_
#define _HMC_LINK_BUF_H_

#include <cstdint>
#include <list>
#include <utility>

class hmc_notify;
class hmc_link;

class hmc_link_fifo {
private:
#ifdef HMC_LOGGING
  uint64_t *cur_cycle;
  hmc_link *link;
#endif /* #ifdef HMC_LOGGING */

  unsigned bitoccupation;
  unsigned bitoccupationmax;
  std::list< std::pair<char*,unsigned> > buf;
#ifdef HMC_USES_NOTIFY
  hmc_notify *notify;
#endif /* #ifdef HMC_USES_NOTIFY */

public:
  explicit hmc_link_fifo(uint64_t *cycle, hmc_notify *notify, hmc_link *link);
  ~hmc_link_fifo(void);

  void adjust_size(unsigned bitsize);

  bool reserve_space(unsigned packetleninbit);
  void push_back_set_avail(char *packet, unsigned packetleninbit);

  char *front(unsigned *packetleninbit);
  void pop_front(void);
};

#endif /* #ifndef _HMC_LINK_BUF_H_ */
