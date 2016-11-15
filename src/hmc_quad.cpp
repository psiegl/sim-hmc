#include "hmc_quad.h"
#include "hmc_cube.h"
#ifdef HMC_USES_BOBSIM
#include "hmc_bobsim.h"
#else
#include "hmc_vault.h"
#endif
#include "hmc_notify.h"
#include "hmc_link.h"
#include "hmc_ring.h"

hmc_quad::hmc_quad(unsigned id, hmc_notify *notify,
                   hmc_cube *cube, enum link_width_t vaultbuswidth) :
  hmc_notify_cl(),
  ring_notify(id, notify, this),
  ring(id, &this->ring_notify, cube),
  vault_notify(id, notify, this)
{
  for (unsigned i = 0; i < HMC_NUM_VAULTS / HMC_NUM_QUADS; i++) {
    hmc_link *link = new hmc_link[2];
    link[0].connect_linkports(&link[1]);
    link[0].re_adjust_links(vaultbuswidth, 1);
    this->link_garbage.push_back(link);

#ifdef HMC_USES_BOBSIM
    this->vaults[i] = new hmc_bobsim(i, 1, false, cube, &this->vault_notify, &link[1]);
#else
    this->vaults[i] = new hmc_vault(i, cube, &this->vault_notify, &link[1]);
#endif /* #ifdef HMC_USES_BOBSIM */
    this->ring.set_vault_link(i, &link[0]);
  }
}

hmc_quad::~hmc_quad(void)
{
  for (unsigned i = 0; i < HMC_NUM_VAULTS / HMC_NUM_QUADS; i++) {
    delete this->vaults[i];
  }
  for (auto it = this->link_garbage.begin(); it != this->link_garbage.end(); ++it)
    delete[] *it;
}

void hmc_quad::clock(void)
{
  uint32_t notifymap = this->vault_notify.get_notification();
  if (notifymap) {
    unsigned lid = __builtin_ctzl(notifymap);
    for (unsigned v = lid; v < HMC_NUM_VAULTS / HMC_NUM_QUADS; v++) {
      if ((0x1 << v) & notifymap) {
        this->vaults[v]->clock();
      }
    }
  }

  if (this->ring_notify.get_notification())
    this->ring.clock();
}

bool hmc_quad::notify_up(void)
{
  return (!this->vault_notify.get_notification() &&
          !this->ring_notify.get_notification());
}

bool hmc_quad::set_ext_link(hmc_link *link)
{
  return this->ring.set_ext_link(link);
}

bool hmc_quad::set_ring_link(unsigned id, hmc_link *link)
{
  return this->ring.set_ring_link(id, link);
}
