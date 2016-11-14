#ifndef _HMC_QUAD_H_
#define _HMC_QUAD_H_

#include <list>
#include "hmc_ring.h"
#include "hmc_notify.h"
#include "config.h"

#ifdef HMC_USES_BOBSIM
class hmc_bobsim;
#else
class hmc_vault;
#endif /* #ifdef HMC_USES_BOBSIM */
class hmc_cube;
class hmc_link;

class hmc_quad : private hmc_notify_cl {

  hmc_notify ring_notify;
  hmc_ring ring;

  hmc_notify vault_notify;
#ifdef HMC_USES_BOBSIM
  std::map<unsigned, hmc_bobsim*> vaults;
#else
  std::map<unsigned, hmc_vault*> vaults;
#endif /* #ifdef HMC_USES_BOBSIM */
  std::list<hmc_link*> link_garbage;

public:
  hmc_quad(unsigned id, hmc_notify *notify, hmc_cube *cube);
  virtual ~hmc_quad(void);

  void clock(void);
  bool notify_up(void);

  bool set_ext_link(hmc_link* link);
  bool set_ring_link(unsigned id, hmc_link* link);
};

#endif /* #ifndef _HMC_QUAD_H_ */
