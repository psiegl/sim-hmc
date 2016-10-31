#ifndef _HMC_LINK_H_
#define _HMC_LINK_H_

#include "hmc_queue.h"

template <typename Ti, typename To>
class hmc_link {
  hmc_queue<Ti> i;
  hmc_queue<To>* o;

public:
  hmc_link(void);
  ~hmc_link(void);

  void re_adjust(unsigned bitwidth, unsigned queuedepth);
  void set_ilink_notify(unsigned id, Ti* cl, void (Ti::*add)(unsigned id), void (Ti::*del)(unsigned id));
  void set_olink(hmc_queue<To>* olink);
  hmc_queue<Ti>* get_ilink(void);

  hmc_queue<To>* get_olink(void);
};

#include "hmc_link.tcc"

#endif /* #ifndef _HMC_LINK_H_ */
