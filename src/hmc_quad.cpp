#include "hmc_quad.h"
#include "hmc_connection.h"
#include "hmc_cube.h"
#ifdef HMC_USES_BOBSIM
#include "hmc_bobsim.h"
#else
#include "hmc_vault.h"
#endif
#include "hmc_notify.h"
#include "hmc_link.h"

hmc_quad::hmc_quad(unsigned id, hmc_conn_part *conn, unsigned num_ranks, hmc_notify *notify,
                   hmc_cube *cube, uint64_t *clk) :
  hmc_notify_cl(),
  vault_notify(id, notify, this)
{
  for (unsigned i = 0; i < HMC_NUM_VAULTS / HMC_NUM_QUADS; i++) {
#ifdef HMC_USES_BOBSIM
    this->vaults[i] = new hmc_bobsim(i, id, 1, num_ranks, false, cube, &this->vault_notify);
#else
    this->vaults[i] = new hmc_vault(i, cube, &this->vault_notify);
#endif /* #ifdef HMC_USES_BOBSIM */

    hmc_link *linkend0 = new hmc_link(clk, conn, HMC_LINK_VAULT, i);
    hmc_link *linkend1 = new hmc_link(clk, this->vaults[i], HMC_LINK_VAULT, id);
    linkend0->connect_linkports(linkend1);
    /*
     * Vault: bi-directional    80Gbit/s
     *        in one direction: 40Gbit/s (bitwidth: 32bits * bitrate: 1.25Gbit/s)
     */
    linkend0->adjust_both_linkends(32, 1.25f);
    // ToDo: adjust buf!
    this->link_garbage.push_back(linkend0);
    this->link_garbage.push_back(linkend1);
  }
}

hmc_quad::~hmc_quad(void)
{
  for (unsigned i = 0; i < HMC_NUM_VAULTS / HMC_NUM_QUADS; i++)
    delete this->vaults[i];
  for (std::list<hmc_link*>::iterator it = this->link_garbage.begin(); it != this->link_garbage.end(); ++it)
    delete *it;
}

void hmc_quad::clock(void)
{
#ifdef HMC_USES_NOTIFY
  unsigned notifymap = this->vault_notify.get_notification();
  if (notifymap)
#endif /* #ifdef HMC_USES_NOTIFY */
  {
#ifdef HMC_USES_NOTIFY
    unsigned lid = __builtin_ctzl(notifymap);
#else
    unsigned lid = 0;
#endif /* #ifdef HMC_USES_NOTIFY */
    for (unsigned v = lid; v < HMC_NUM_VAULTS / HMC_NUM_QUADS; v++) {
#ifdef HMC_USES_NOTIFY
      if ((0x1 << v) & notifymap)
#endif /* #ifdef HMC_USES_NOTIFY */
      {
        this->vaults[v]->clock();
      }
    }
  }
}

bool hmc_quad::notify_up(void)
{
#ifdef HMC_USES_NOTIFY
  return (!this->vault_notify.get_notification());
#else
  return true;
#endif /* #ifdef HMC_USES_NOTIFY */
}

