#ifndef _HMC_ROUTE_H_
#define _HMC_ROUTE_H_

#include <map>
#include "hmc_queue.h"
#include "hmc_macros.h"

class hmc_route {
  unsigned id;

  std::map<unsigned, std::pair<unsigned,unsigned>> slidToCube;

public:
  hmc_route(unsigned id);
  ~hmc_route(void);

  void set_slid(unsigned slid, unsigned cubId, unsigned quadId);

  ALWAYS_INLINE unsigned slid_to_cubid(unsigned slid)
  {
    return this->slidToCube[slid].first;
  }

  ALWAYS_INLINE unsigned slid_to_quadid(unsigned slid)
  {
    return this->slidToCube[slid].second;
  }

  ALWAYS_INLINE hmc_queue* ext_routing(unsigned cubId, unsigned curQuadId)
  {
    return nullptr;
  }
};

#endif /* #ifndef _HMC_ROUTE_H_ */
