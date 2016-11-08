#include "hmc_cube.h"
#include "hmc_quad.h"

hmc_cube::hmc_cube(unsigned id, hmc_notify *notify) :
  hmc_decode(),
  hmc_route(id),
  hmc_notify_cl(),
  hmc_register(this),
  id(id),
  quad_notify( id, notify, this )
{
  for(unsigned i=0; i<HMC_NUM_QUADS; i++)
    this->quads[i] = new hmc_quad(i, &this->quad_notify, this);
}

hmc_cube::~hmc_cube(void)
{
  for(unsigned i=0; i<HMC_NUM_QUADS; i++)
    delete this->quads[i];
}

void hmc_cube::clock(void)
{
  uint32_t notifymap = this->quad_notify.get_notification();
  if(notifymap)
  {
    unsigned lid = __builtin_ctzl(notifymap);
    for(unsigned q = lid; q < HMC_NUM_QUADS; q++ )
    {
      if((0x1 << q) & notifymap) {
        this->quads[q]->clock();
      }
    }
  }
}

bool hmc_cube::notify_up(void)
{
  return (!this->quad_notify.get_notification());
}
