#ifndef _HMC_CONNECTION_H_
#define _HMC_CONNECTION_H_

#include <array>
#include "config.h"
#include "hmc_notify.h"
#include "hmc_macros.h"
#include "hmc_module.h"

class hmc_cube;
class hmc_quad;
class hmc_link;

#define HMC_JTL_ALL_LINKS         ( HMC_MAX_LINKS/HMC_NUM_QUADS + HMC_NUM_QUADS + HMC_NUM_VAULTS / HMC_NUM_QUADS )
#define HMC_JTL_EXT_LINK( x )     ( x )
#define HMC_JTL_RING_LINK( x )    ( HMC_MAX_LINKS/HMC_NUM_QUADS + (x) )
#define HMC_JTL_VAULT_LINK( x )   ( HMC_MAX_LINKS/HMC_NUM_QUADS + HMC_NUM_QUADS + (x) )

class hmc_conn_part : private hmc_notify_cl, public hmc_module {
protected:
  unsigned id;

private:
  hmc_cube* cub;

  hmc_notify links_notify;
  hmc_notify linkrxbuf_notify;
  std::array<hmc_link*, HMC_JTL_ALL_LINKS> links;
  unsigned roundRobinSchedule;
  unsigned cyclesBlocked;

  unsigned decode_link_of_packet(char* packet);
  bool _set_link(unsigned notifyid, unsigned id, hmc_link *link);

  virtual unsigned routing(unsigned nextquad) = 0;

  bool notify_up(unsigned id);

public:
  hmc_conn_part(unsigned id, hmc_notify *notify, hmc_cube* cub);
  virtual ~hmc_conn_part(void);

  bool set_link(unsigned linkId, hmc_link* link, enum hmc_link_type linkType)
  {
    switch(linkType) {
    case HMC_LINK_RING:
      return this->_set_link(HMC_JTL_RING_LINK(linkId), this->id, link);
    case HMC_LINK_VAULT_IN:
    case HMC_LINK_VAULT_OUT:
      return this->_set_link(HMC_JTL_VAULT_LINK(linkId), this->id, link);
    case HMC_LINK_EXTERN:
    case HMC_LINK_SLID:
      return this->_set_link(HMC_JTL_EXT_LINK(linkId), this->id, link);
    }
    return false;
  }

  void clock(void);
  unsigned get_id(void) { return this->id; }
};

class hmc_conn : public hmc_notify_cl {
protected:
  hmc_notify conn_notify;
  std::array<hmc_conn_part*, HMC_NUM_QUADS> conns;
  std::list<hmc_link*> link_garbage;

private:
  bool notify_up(unsigned id);

public:
  hmc_conn(hmc_notify *notify) :
    conn_notify(0, notify, this)
  {}
  virtual ~hmc_conn(void) {
    for (auto it = this->link_garbage.begin(); it != this->link_garbage.end(); ++it)
      delete *it;
  }

  ALWAYS_INLINE hmc_conn_part* get_conn(unsigned id) {
    return this->conns[id];
  }

  void clock(void);
};

#endif /* #ifndef _HMC_CONNECTION_H_ */
