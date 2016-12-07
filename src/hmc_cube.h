#ifndef _HMC_CUBE_H_
#define _HMC_CUBE_H_

#include <vector>
#include <list>
#include "hmc_macros.h"
#include "hmc_route.h"
#include "hmc_notify.h"
#include "hmc_register.h"

class hmc_quad;
class hmc_link;

class hmc_cube : public hmc_route,
                 private hmc_notify_cl,
                 public hmc_register {
private:
  unsigned id;

  hmc_notify quad_notify;
  std::vector<hmc_quad*> quads;

  std::list<hmc_link*> link_garbage;

  bool notify_up(void);

public:
  hmc_cube(unsigned id, hmc_notify *notify,
           unsigned ringbus_bitwidth, float ringbus_bitrate,
           unsigned capacity,
           std::map<unsigned, hmc_cube*>* cubes, unsigned numcubes, uint64_t *clk);
  virtual ~hmc_cube(void);

  ALWAYS_INLINE unsigned get_id(void)
  {
    return this->id;
  }

  ALWAYS_INLINE hmc_quad* get_quad(unsigned id)
  {
    return this->quads[id];
  }

  void clock(void);
};


#endif /* #ifndef _HMC_CUBE_H_ */
