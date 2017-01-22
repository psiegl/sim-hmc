#ifndef _HMC_LINK_BUF_H_
#define _HMC_LINK_BUF_H_

#include <list>
#include <utility>
#include <stdint.h>

class hmc_notify;
class hmc_link;

class hmc_link_buf {
private:
#ifdef HMC_LOGGING
  uint64_t *cur_cycle;
  hmc_link *link;
#endif /* #ifdef HMC_LOGGING */

  float bitoccupation;
  unsigned bitoccupationmax;
  std::list< std::pair<char*,unsigned> > buf;

  hmc_notify *notify;

public:
  explicit hmc_link_buf(uint64_t *cycle, hmc_notify *notify, hmc_link *link);
  ~hmc_link_buf(void);

  void adjust_size(unsigned bitsize);

  bool reserve_space(float packetleninbit);
  void push_back_set_avail(char *packet, unsigned packetleninbit);

  char *front(unsigned *packetleninbit);
  void pop_front(void);
};

#endif /* #ifndef _HMC_LINK_BUF_H_ */
