#include <cstdlib>
#include <iostream>
#include "hmc_cube.h"
#include "hmc_quad.h"
#include "hmc_link.h"
#include "hmc_conn_ring.h"
#include "hmc_conn_xbar.h"

hmc_cube::hmc_cube(unsigned id, hmc_notify *notify,
                   unsigned quadbus_bitwidth, float quadbus_bitrate,
                   unsigned capacity,
                   std::map<unsigned, hmc_cube*> *cubes, unsigned numcubes, uint64_t *clk) :
  hmc_route(cubes, numcubes),
  hmc_notify_cl(),
  hmc_register(this, capacity),
  id(id),
  quad_notify(id, notify, this),
  conn_notify(id, notify, this),
  conn(nullptr)
{
  char *quadConnection = getenv("HMCSIM_QUAD_CONNECTION");
  // default is ring to connection quads
  if (quadConnection == nullptr || !strcmp("ring", quadConnection))
    this->conn = new hmc_ring(&this->conn_notify, this, quadbus_bitwidth, quadbus_bitrate, clk);
  else if (!strcmp("xbar", quadConnection))
    this->conn = new hmc_xbar(&this->conn_notify, this, quadbus_bitwidth, quadbus_bitrate, clk);
  else {
    std::cerr << "ERROR: env HMCSIM_QUAD_CONNECTION has wrong value! " << quadConnection << ", choose ring or xbar" << std::endl;
    exit(-1);
  }

  unsigned num_ranks = capacity; /* num_ranks 8GB -> 8 layer, 4GB -> 4layer */
  for (unsigned i = 0; i < HMC_NUM_QUADS; i++)
    this->quads[i] = new hmc_quad(i, this->conn->get_conn(i), num_ranks, &this->quad_notify, this, clk);
}

hmc_cube::~hmc_cube(void)
{
  for (unsigned i = 0; i < HMC_NUM_QUADS; i++)
    delete this->quads[i];
  delete this->conn;
}

void hmc_cube::clock(void)
{
#ifdef HMC_USES_NOTIFY
  if (this->conn_notify.get_notification())
#endif
  {
    this->conn->clock();
  }

#ifdef HMC_USES_NOTIFY
  unsigned notifymap = this->quad_notify.get_notification();
  for (unsigned i, lid = i = __builtin_ctzl(notifymap);
       notifymap >>= lid;
       lid = __builtin_ctzl(notifymap >>= 1),
       i += (lid + 1))
#else
  for (unsigned i = 0; i < HMC_NUM_QUADS; i++)
#endif /* #ifdef HMC_USES_NOTIFY */
  {
    this->quads[i]->clock();
  }
}

bool hmc_cube::notify_up(unsigned id)
{
#ifdef HMC_USES_NOTIFY
  return (!this->conn_notify.get_notification()
          && !this->quad_notify.get_notification());
#else
  return true;
#endif /* #ifdef HMC_USES_NOTIFY */
}
