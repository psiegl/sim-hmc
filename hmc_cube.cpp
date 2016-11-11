#include "hmc_cube.h"
#include "hmc_quad.h"

hmc_cube::hmc_cube(hmc_sim *sim, unsigned id, hmc_notify *notify) :
  hmc_route(sim),
  hmc_notify_cl(),
  hmc_register(this), // register needs to be before decode!
  hmc_decode(this->hmcsim_util_get_bsize(), this->hmcsim_util_get_num_banks_per_vault()),
  id(id),
  quad_notify( id, notify, this )
{
  for(unsigned i=0; i<HMC_NUM_QUADS; i++)
  {
    this->quads[i] = new hmc_quad(i, &this->quad_notify, this);
  }

  for(unsigned i=0; i<HMC_NUM_QUADS; i++)
  {
    unsigned map[] = { 0x2, 0x0, 0x3, 0x1 };

    std::cout << "connecting " << i << " and " << map[i] << std::endl;
    unsigned neighbour =map[i];
    hmc_quad * quad0 = this->quads[i];
    hmc_quad * quad1 = this->quads[neighbour];
    hmc_link *links = new hmc_link[2];
    links[0].connect_linkports(&links[1]);
    links[0].re_adjust_links(64,1);
    quad0->set_ring_link(neighbour, &links[0]);
    quad1->set_ring_link(i, &links[1]);
  }
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
