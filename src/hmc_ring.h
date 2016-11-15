#ifndef _HMC_RING_H_
#define _HMC_RING_H_

#include <map>
#include "config.h"
#include "hmc_notify.h"
#include "hmc_macros.h"

class hmc_cube;
class hmc_link;
class hmc_queue;
class hmc_quad;

#define HMC_JTL_ALL_LINKS         ( 1 + HMC_NUM_QUADS + HMC_NUM_VAULTS / HMC_NUM_QUADS )
#define HMC_JTL_EXT_LINK          ( 0 )
#define HMC_JTL_RING_LINK( x )    ( 1 + (x) )
#define HMC_JTL_VAULT_LINK( x )   ( 1 + HMC_NUM_QUADS + (x) )

class hmc_ring : private hmc_notify_cl {
private:
  unsigned id;
  hmc_cube* cub;

  hmc_notify links_notify;
  std::map<unsigned, hmc_link*> links;

  unsigned decode_link_of_packet(void* packet);
  bool set_link(unsigned id, hmc_link *link);

public:
  hmc_ring(unsigned id, hmc_notify *notify, hmc_cube* cub);
  ~hmc_ring(void);

  ALWAYS_INLINE bool set_ring_link(unsigned id, hmc_link* link)
  {
    return this->set_link(HMC_JTL_RING_LINK(id), link);
  }

  ALWAYS_INLINE bool set_vault_link(unsigned id, hmc_link* link)
  {
    return this->set_link(HMC_JTL_VAULT_LINK(id), link);
  }

  ALWAYS_INLINE bool set_ext_link(hmc_link* link)
  {
    return this->set_link(HMC_JTL_EXT_LINK, link);
  }

  void clock(void);

  bool notify_up(void);
};

#endif /* #ifndef _HMC_RING_H_ */
