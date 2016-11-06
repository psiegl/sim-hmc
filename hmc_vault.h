#ifndef _HMC_VAULT_H_
#define _HMC_VAULT_H_

#include "hmc_vault.h"
#include "hmc_link.h"
#include "crossbar.h"

template <typename t>
class hmc_xbar<T>;

template <typename T>
class hmc_vault {

  unsigned id;

  unsigned needs_clock;
  hmc_link< hmc_vault<T>, hmc_xbar<T> > link;

  void notify_add(unsigned id);
  void notify_del(unsigned id);

public:
  hmc_vault(unsigned id);
  ~hmc_vault(void);

};

#include "hmc_vault.tcc"

#endif /* #ifndef _HMC_VAULT_H_ */
