#ifndef _HMC_RING_H_
#define _HMC_RING_H_

#include <map>
#include "hmc_link.h"
#include "hmc_notify.h"
#include "config.h"

class hmc_ring {
private:
  unsigned id;

  hmc_notify ring_notify;
  std::map<unsigned, hmc_link*> ring_link;

  hmc_notify vault_notify;
  std::map<unsigned, hmc_link*> vault_link;

  hmc_notify ext_notify;
  hmc_link* ext_link; // ToDo

public:
  hmc_ring(unsigned id, hmc_notify *notify);
  ~hmc_ring(void);

  int set_ring_link(unsigned id, hmc_link* link);
  int set_vault_link(unsigned id, hmc_link* link);
  int set_ext_link(unsigned id, hmc_link* link);

  void clock(void);
};

#endif /* #ifndef _HMC_RING_H_ */
