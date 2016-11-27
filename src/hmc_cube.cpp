#include "hmc_cube.h"
#include "hmc_quad.h"
#include "hmc_link.h"

hmc_cube::hmc_cube(unsigned id, hmc_notify *notify,
                   enum link_width_t ringbuswidth, enum link_width_t vaultbuswidth,
                   unsigned capacity,
                   std::map<unsigned, hmc_cube*> *cubes, unsigned numcubes) :
  hmc_route(cubes, numcubes),
  hmc_notify_cl(),
  hmc_register(this, capacity),
  id(id),
  quad_notify(id, notify, this)
{
  for (unsigned i = 0; i < HMC_NUM_QUADS; i++) {
    this->quads[i] = new hmc_quad(i, &this->quad_notify, this, vaultbuswidth);
  }

  for (unsigned i = 0; i < HMC_NUM_QUADS; i++) {
    unsigned map[] = { 0x2, 0x0, 0x3, 0x1 };

    unsigned neighbour = map[i];
    hmc_quad *quad0 = this->quads[i];
    hmc_quad *quad1 = this->quads[neighbour];
    hmc_link *links = new hmc_link[2];
    links[0].connect_linkports(&links[1]);
    links[0].re_adjust_links(ringbuswidth, 1);
    quad0->set_ring_link(neighbour, &links[0]);
    quad1->set_ring_link(i, &links[1]);
    this->link_garbage.push_back(links);
  }
}

hmc_cube::~hmc_cube(void)
{
  for (unsigned i = 0; i < HMC_NUM_QUADS; i++)
    delete this->quads[i];
  for (auto it = this->link_garbage.begin(); it != this->link_garbage.end(); ++it)
    delete[] *it;
}

void hmc_cube::clock(void)
{
  uint32_t notifymap = this->quad_notify.get_notification();
  if (notifymap) {
    unsigned lid = __builtin_ctzl(notifymap);
    for (unsigned q = lid; q < HMC_NUM_QUADS; q++) {
      if ((0x1 << q) & notifymap) {
        this->quads[q]->clock();
      }
    }
  }
}

bool hmc_cube::notify_up(void)
{
  return (!this->quad_notify.get_notification());
}
