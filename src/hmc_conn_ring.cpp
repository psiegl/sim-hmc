#include "hmc_conn_ring.h"
#include "hmc_notify.h"
#include "hmc_cube.h"
#include "hmc_link.h"

hmc_ring::hmc_ring(hmc_notify *notify, hmc_cube *cub, unsigned ringbus_bitwidth, float ringbus_bitrate, uint64_t *clk) :
  hmc_conn(notify)
{
  for (unsigned i = 0; i < HMC_NUM_QUADS; i++)
    this->conns[i] = new hmc_ring_part(i, &this->conn_notify, cub);

  // first create conns above so that there is no nullptr conflict when connecting them below
  unsigned map[] = { 0x2, 0x0, 0x3, 0x1 };
  for (unsigned i = 0; i < HMC_NUM_QUADS; i++) {
    unsigned neighbour = map[i];
    hmc_link *linkend0 = new hmc_link(clk, HMC_LINK_RING, this->conns[i], cub, neighbour);
    hmc_link *linkend1 = new hmc_link(clk, HMC_LINK_RING, this->conns[neighbour], cub, i);
    linkend0->connect_linkports(linkend1);
    linkend0->adjust_both_linkends(ringbus_bitwidth, ringbus_bitrate, FLIT_WIDTH * RETRY_BUFFER_FLITS);
    this->link_garbage.push_back(linkend0);
    this->link_garbage.push_back(linkend1);
  }
}

hmc_ring::~hmc_ring(void)
{
  for (unsigned i = 0; i < HMC_NUM_QUADS; i++)
    delete this->conns[i];
}


