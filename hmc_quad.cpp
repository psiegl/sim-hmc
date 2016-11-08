#include "hmc_quad.h"
#include "hmc_cube.h"
#include "hmc_vault.h"
#include "hmc_notify.h"
#include "hmc_link.h"
#include "hmc_ring.h"

hmc_quad::hmc_quad(unsigned id, hmc_notify *notify, hmc_cube *cub) :
  hmc_notify_cl(),
  ring_notify(id, notify, this),
  ring(id, &this->ring_notify, cub),
  vault_notify(id, notify, this)
{
  for(unsigned i=0; i< HMC_NUM_VAULTS / HMC_NUM_QUADS; i++)
  {
    hmc_link *link = new hmc_link[2];
    link[0].connect_linkports(&link[1]);
    link[0].re_adjust_links(64, 1); // ToDo
    this->link_garbage.push_back(link);

    this->vaults[i] = new hmc_vault(i, &this->vault_notify, &link[1]);
    this->ring.set_vault_link(i, &link[0]);
  }
}

hmc_quad::~hmc_quad(void)
{
  for(unsigned i=0; i< HMC_NUM_VAULTS / HMC_NUM_QUADS; i++)
  {
    delete this->vaults[i];
  }
  std::list<hmc_link*>::iterator it;
  for(it = this->link_garbage.begin(); it != this->link_garbage.end(); ++it)
    delete[] *it;
}

void hmc_quad::clock(void)
{
  uint32_t notifymap = this->vault_notify.get_notification();
  if(notifymap)
  {
    unsigned lid = __builtin_ctzl(notifymap);
    for(unsigned v = lid; v < HMC_NUM_VAULTS / HMC_NUM_QUADS; v++ )
    {
      if((0x1 << v) & notifymap) {
        this->vaults[v]->clock();
      }
    }
  }

  if(this->ring_notify.get_notification())
    this->ring.clock();
}

bool hmc_quad::notify_up(void) {
  return (!this->vault_notify.get_notification() &&
          !this->ring_notify.get_notification());
}

bool hmc_quad::set_ext_link(hmc_link* link)
{
  return this->ring.set_ext_link(link);
}
