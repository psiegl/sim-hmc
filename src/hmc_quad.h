#ifndef _HMC_QUAD_H_
#define _HMC_QUAD_H_

#include <list>
#include <array>
#include "config.h"
#include "hmc_notify.h"

#ifdef HMC_USES_BOBSIM
class hmc_bobsim;
#else
class hmc_vault;
#endif /* #ifdef HMC_USES_BOBSIM */
class hmc_cube;
class hmc_conn_part;
class hmc_link;

class hmc_quad : private hmc_notify_cl {
private:
  hmc_notify vault_notify;
#ifdef HMC_USES_BOBSIM
  std::array<hmc_bobsim*, HMC_NUM_VAULTS / HMC_NUM_QUADS> vaults;
#else
  std::array<hmc_vault*, HMC_NUM_VAULTS / HMC_NUM_QUADS> vaults;
#endif /* #ifdef HMC_USES_BOBSIM */
  std::list<hmc_link*> link_garbage;

  bool notify_up(void);

public:
  hmc_quad(unsigned id, hmc_conn_part *conn, unsigned num_ranks, hmc_notify *notify,
           hmc_cube *cube, uint64_t *clk);
  virtual ~hmc_quad(void);

  void clock(void);
};

#endif /* #ifndef _HMC_QUAD_H_ */
