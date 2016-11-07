#ifndef _HMC_QUAD_H_
#define _HMC_QUAD_H_

#include "hmc_ring.h"

class hmc_quad : public hmc_ring {
public:
  hmc_quad(unsigned id, hmc_notify *notify, hmc_cube *cub);
  ~hmc_quad(void);
};

#endif /* #ifndef _HMC_QUAD_H_ */
