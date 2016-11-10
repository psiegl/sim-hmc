#ifndef _HMC_CUBE_H_
#define _HMC_CUBE_H_

#include <map>
#include <cmath>
#include "hmc_macros.h"
#include "hmc_decode.h"
#include "hmc_route.h"
#include "hmc_notify.h"
#include "hmc_register.h"

class hmc_quad;
class hmc_sim;

class hmc_cube : public hmc_route, private hmc_notify_cl, public hmc_register, public hmc_decode {
private:
  unsigned id;

  hmc_notify quad_notify;
  std::map<unsigned, hmc_quad*> quads;

public:
  hmc_cube(hmc_sim *sim, unsigned id, hmc_notify *notify);
  ~hmc_cube(void);

  ALWAYS_INLINE unsigned get_id(void)
  {
    return this->id;
  }

  ALWAYS_INLINE hmc_quad* get_quad(unsigned id)
  {
    return this->quads[id];
  }

  void clock(void);

  bool notify_up(void);
};


#endif /* #ifndef _HMC_CUBE_H_ */
