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

hmc_quad::hmc_quad(unsigned id, unsigned num_ranks, hmc_notify *notify,
                   hmc_cube *cube, uint64_t *clk) :
  hmc_notify_cl(),
  ring_notify(id, notify, this),
  ring(id, &this->ring_notify, cube),
  vault_notify(id, notify, this)
{
  for (unsigned i = 0; i < HMC_NUM_VAULTS / HMC_NUM_QUADS; i++) {
    hmc_link *linkend0 = new hmc_link(clk);
    hmc_link *linkend1 = new hmc_link(clk);
    linkend0->connect_linkports(linkend1);
    linkend0->re_adjust_links(HMCSIM_FULL_LINK_WIDTH, 1);
    //linkend0->re_adjust_links(32, 1.25f); // ToDo
    this->link_garbage.push_back(linkend0);
    this->link_garbage.push_back(linkend1);

#ifdef HMC_USES_BOBSIM
    this->vaults[i] = new hmc_bobsim(i, id, 1, num_ranks, false, cube, &this->vault_notify, linkend1);
#else
    this->vaults[i] = new hmc_vault(i, cube, &this->vault_notify, linkend1);
#endif /* #ifdef HMC_USES_BOBSIM */
    this->ring.set_vault_link(i, linkend0);
  }
}

hmc_quad::~hmc_quad(void)
{
  for (unsigned i = 0; i < HMC_NUM_VAULTS / HMC_NUM_QUADS; i++) {
    delete this->vaults[i];
  }
  for (std::list<hmc_link*>::iterator it = this->link_garbage.begin(); it != this->link_garbage.end(); ++it)
    delete *it;
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
