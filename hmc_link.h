#ifndef _HMC_LINK_H_
#define _HMC_LINK_H_

#include "hmc_queue.h"
#include "hmc_notify.h"


class hmc_link {
  hmc_queue i;
  hmc_queue *o;

  hmc_link *binding;

public:
  hmc_link(void);
  ~hmc_link(void);

  hmc_queue* get_ilink(void);
  hmc_queue* get_olink(void);

  void set_ilink_notify(unsigned id, hmc_notify *notify);

  void re_adjust_links(unsigned bitwidth, unsigned queuedepth);

  // setup of two parts of hmc_link to form ONE link
  void connect_linkports(hmc_link *part);
  void set_binding(hmc_link* part);
};

#endif /* #ifndef _HMC_LINK_H_ */
