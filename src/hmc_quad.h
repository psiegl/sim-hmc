#ifndef _HMC_QUAD_H_
#define _HMC_QUAD_H_

#include <list>
#include <map>
#include "config.h"
#include "hmc_notify.h"

#ifdef HMC_USES_BOBSIM
class hmc_bobsim;
#else
class hmc_vault;
#endif /* #ifdef HMC_USES_BOBSIM */
class hmc_cube;
class hmc_connection;
class hmc_link;

class hmc_quad : private hmc_notify_cl {
private:
  hmc_notify vault_notify;
#ifdef HMC_USES_BOBSIM
  std::map<unsigned, hmc_bobsim*> vaults;
#else
  std::map<unsigned, hmc_vault*> vaults;
#endif /* #ifdef HMC_USES_BOBSIM */
  std::list<hmc_link*> link_garbage;

  bool notify_up(void);

public:
  hmc_quad(unsigned id, hmc_connection *conn, unsigned num_ranks, hmc_notify *notify,
           hmc_cube *cube, uint64_t *clk);
  virtual ~hmc_quad(void);

  void clock(void);
};

#endif /* #ifndef _HMC_QUAD_H_ */
