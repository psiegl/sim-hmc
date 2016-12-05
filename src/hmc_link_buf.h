#ifndef _HMC_LINK_BUF_H_
#define _HMC_LINK_BUF_H_

class hmc_link_buf {

  unsigned bitoccupation;
  unsigned bitoccupationmax;

public:
  hmc_link_buf(unsigned bitsize);
  ~hmc_link_buf(void);

  bool reserve_space(unsigned packetleninbit);
  void push_back_set_avail(char *packet, unsigned *packetleninbit);

  char *front(unsigned *packetleninbit);
  char *pop_front(void);
};

#endif /* #ifndef _HMC_LINK_BUF_H_ */
