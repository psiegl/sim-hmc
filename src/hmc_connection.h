#ifndef _HMC_CONNECTION_H_
#define _HMC_CONNECTION_H_

#include <vector>
#include "config.h"
#include "hmc_notify.h"
#include "hmc_macros.h"
#include "hmc_link.h"

class hmc_cube;
class hmc_quad;

#define HMC_JTL_ALL_LINKS         ( HMC_MAX_LINKS/HMC_NUM_QUADS + HMC_NUM_QUADS + HMC_NUM_VAULTS / HMC_NUM_QUADS )
#define HMC_JTL_EXT_LINK( x )     ( x )
#define HMC_JTL_RING_LINK( x )    ( HMC_MAX_LINKS/HMC_NUM_QUADS + (x) )
#define HMC_JTL_VAULT_LINK( x )   ( HMC_MAX_LINKS/HMC_NUM_QUADS + HMC_NUM_QUADS + (x) )

class hmc_connection : private hmc_notify_cl {
protected:
  unsigned id;
private:
  hmc_cube* cub;

  hmc_notify links_notify;
  std::vector<hmc_link*> links;

  unsigned decode_link_of_packet(char* packet);
  bool _set_link(unsigned notifyid, unsigned id, hmc_link *link);

  virtual unsigned routing(unsigned nextquad) = 0;

  bool notify_up(void);

public:
  hmc_connection(unsigned id, hmc_notify *notify, hmc_cube* cub);
  virtual ~hmc_connection(void);

  bool set_link(unsigned linkId, hmc_link* link, enum hmc_link_type linkType)
  {
    switch(linkType) {
    case HMC_LINK_RING:
      return this->_set_link(HMC_JTL_RING_LINK(linkId), this->id, link);
    case HMC_LINK_VAULT:
      return this->_set_link(HMC_JTL_VAULT_LINK(linkId), this->id, link);
    case HMC_LINK_EXTERN:
      return this->_set_link(HMC_JTL_EXT_LINK(linkId), this->id, link);
    case HMC_LINK_UNDEFINED:
      break;
    }
    return false;
  }

  void clock(void);
};

#endif /* #ifndef _HMC_CONNECTION_H_ */
