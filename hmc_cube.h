#ifndef _HMC_CUBE_H_
#define _HMC_CUBE_H_

#include <map>
#include "hmc_decode.h"
#include "hmc_route.h"
#include "hmc_notify.h"

class hmc_quad;

class hmc_cube : public hmc_decode, public hmc_route {
private:
  unsigned id;

  hmc_notify quad_notify;
  std::map<unsigned, hmc_quad*> quads;

public:
  hmc_cube(unsigned id, hmc_notify *notify);
  ~hmc_cube(void);

  unsigned get_id(void);
  hmc_quad* get_quad(unsigned id)
  {
    return this->quads[id];
  }

  void clock(void);
};


#endif /* #ifndef _HMC_CUBE_H_ */
