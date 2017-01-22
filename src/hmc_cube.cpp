#include "hmc_cube.h"
#include "hmc_quad.h"
#include "hmc_link.h"
#include "hmc_conn_ring.h"
#include "hmc_conn_xbar.h"

hmc_cube::hmc_cube(unsigned id, hmc_notify *notify,
                   unsigned ringbus_bitwidth, float ringbus_bitrate,
                   unsigned capacity,
                   std::map<unsigned, hmc_cube*> *cubes, unsigned numcubes, uint64_t *clk) :
  hmc_route(cubes, numcubes),
  hmc_notify_cl(),
  hmc_register(this, capacity),
  id(id),
  quad_notify(id, notify, this),
  quads(HMC_NUM_QUADS, nullptr),
  conn_notify(id, notify, this),
  conn(nullptr)
{
  this->conn = new hmc_ring(&this->conn_notify, this, ringbus_bitwidth, ringbus_bitrate, clk);

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
  if (notifymap)
#endif
  {
#ifdef HMC_USES_NOTIFY
    unsigned lid = __builtin_ctzl(notifymap);
#else
    unsigned lid = 0;
#endif /* #ifdef HMC_USES_NOTIFY */
    for (unsigned q = lid; q < HMC_NUM_QUADS; q++) {
#ifdef HMC_USES_NOTIFY
      if ((0x1 << q) & notifymap)
#endif /* #ifdef HMC_USES_NOTIFY */
      {
        this->quads[q]->clock();
      }
    }
  }
}

bool hmc_cube::notify_up(void)
{
#ifdef HMC_USES_NOTIFY
  return (!this->conn_notify.get_notification() &&
          !this->quad_notify.get_notification());
#else
  return true;
#endif /* #ifdef HMC_USES_NOTIFY */
}
