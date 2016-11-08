#ifndef _HMC_VAULT_H_
#define _HMC_VAULT_H_

#include "hmc_notify.h"

class hmc_link;

class hmc_vault {

  unsigned id;
  hmc_link *link;

public:
  hmc_vault(unsigned id, const hmc_notify* notify, const hmc_link *link);
  ~hmc_vault(void);

  void clock(void);
};

#endif /* #ifndef _HMC_VAULT_H_ */
