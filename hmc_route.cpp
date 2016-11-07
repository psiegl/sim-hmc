#include "hmc_route.h"

hmc_route::hmc_route(unsigned id) :
  id(id)
{

}

hmc_route::~hmc_route(void)
{

}

void hmc_route::set_slid(unsigned slid, unsigned cubId, unsigned quadId)
{
  this->slidToCube[slid] = std::make_pair(cubId, quadId);
}
