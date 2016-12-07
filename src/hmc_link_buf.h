#ifndef _HMC_LINK_BUF_H_
#define _HMC_LINK_BUF_H_

#include <list>
#include <utility>

class hmc_notify;

class hmc_link_buf {

  float bitoccupation;
  unsigned bitoccupationmax;
  std::list< std::pair<char*,unsigned> > buf;

  hmc_notify *notify;

public:
  hmc_link_buf(hmc_notify *notify);
  ~hmc_link_buf(void);

  void adjust_size(unsigned bitsize);

  bool reserve_space(float packetleninbit);
  void push_back_set_avail(char *packet, unsigned packetleninbit);

  char *front(unsigned *packetleninbit);
  void pop_front(void);
};

#endif /* #ifndef _HMC_LINK_BUF_H_ */
