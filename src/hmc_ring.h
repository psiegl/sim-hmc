#ifndef _HMC_RING_H_
#define _HMC_RING_H_

#include <map>
#include "config.h"
#include "hmc_notify.h"

class hmc_cube;
class hmc_link;
class hmc_queue;
class hmc_quad;

#define HMC_JTL_EXT_LINK()        ( 0 )
#define HMC_JTL_RING_LINK( x )    ( 1 + (x) )
#define HMC_JTL_VAULT_LINK( x )   ( 3 + (x) )

class hmc_ring : private hmc_notify_cl {
private:
  unsigned id;
  hmc_cube* cub;

  hmc_notify ringlinks_notify;
  std::map<unsigned, hmc_link*> ring_link;

  hmc_notify vaultlinks_notify;
  std::map<unsigned, hmc_link*> vault_link;

  hmc_notify extlinks_notify;
  hmc_link* ext_link;

  hmc_link* decode_link_of_packet(void* packet);

public:
  hmc_ring(unsigned id, hmc_notify *notify, hmc_cube* cub);
  ~hmc_ring(void);

  int set_ring_link(unsigned id, hmc_link* link);
  int set_vault_link(unsigned id, hmc_link* link);
  bool set_ext_link(hmc_link* link);

  void clock(void);

  bool notify_up(void);
};

#endif /* #ifndef _HMC_RING_H_ */
