#include <iostream>
#include "hmc_sim.h"
#include "hmc_link.h"
#include "hmc_quad.h"

hmc_sim::hmc_sim(unsigned num_hmcs, unsigned num_slids,
                 unsigned num_links, unsigned capacity,
                 enum link_width_t ringbuswidth,
                 enum link_width_t vaultbuswidth) :
  clk(0),
  cubes_notify(0, nullptr, this)
{
  if ((num_hmcs > HMC_MAX_DEVS) || (!num_hmcs)) {
    std::cerr << "INSUFFICIENT NUMBER DEVICES: between 1 to " << HMC_MAX_DEVS << " (" << num_hmcs << ")" << std::endl;
//    return HMC_ERROR_PARAMS;
  }

  switch (num_links) {
  case HMC_MIN_LINKS:
  case HMC_MAX_LINKS:
    break;
  default:
    std::cerr << "INSUFFICIENT NUMBER LINKS: between " << HMC_MIN_LINKS << " to " << HMC_MAX_LINKS << " (" << num_links << ")" << std::endl;
//    return HMC_ERROR_PARAMS;
  }

  switch (num_slids) {
  case HMC_MIN_LINKS:
  case HMC_MAX_LINKS:
    break;
  default:
    std::cerr << "INSUFFICIENT NUMBER SLIDS: between " << HMC_MIN_LINKS << " to " << HMC_MAX_LINKS << " (" << num_links << ")" << std::endl;
//    return HMC_ERROR_PARAMS;
  }

  switch (capacity) {
  case HMC_MIN_CAPACITY:
  case HMC_MAX_CAPACITY:
    break;
  default:
    std::cerr << "INSUFFICIENT AMOUNT CAPACITY: between " << HMC_MIN_CAPACITY << " to " << HMC_MAX_CAPACITY << " (" << capacity << ")" << std::endl;
//    return HMC_ERROR_PARAMS;
  }

  for (unsigned i = 0; i < num_slids; i++) {
    this->slidnotify[i] = new hmc_notify(i, nullptr, nullptr);
  }

  for (unsigned i = 0; i < num_hmcs; i++) {
    this->cubes[i] = new hmc_cube(this, i, &this->cubes_notify, ringbuswidth, vaultbuswidth);
  }
}

hmc_sim::~hmc_sim(void)
{
  for (auto it = this->slidnotify.begin(); it != this->slidnotify.end(); ++it) {
    delete (*it).second;
  }

  for (auto it = this->cubes.begin(); it != this->cubes.end(); ++it) {
    delete (*it).second;
  }

  for (auto it = this->link_garbage.begin(); it != this->link_garbage.end(); ++it) {
    delete[] *it;
  }
}

bool hmc_sim::notify_up(void)
{
  return true;
}

bool hmc_sim::hmc_link_config(unsigned src_hmcId, unsigned src_linkId,
                              unsigned dst_hmcId, unsigned dst_linkId,
                              enum link_width_t bitwidth)
{
  hmc_link *link = new hmc_link[2];
  link[0].connect_linkports(&link[1]);
  link[0].re_adjust_links(bitwidth, 1);

  hmc_quad *src_quad = this->cubes[src_hmcId]->get_quad(src_linkId);
  hmc_quad *dst_quad = this->cubes[dst_hmcId]->get_quad(dst_linkId);

  bool ret = src_quad->set_ext_link(&link[0]);
  ret &= dst_quad->set_ext_link(&link[1]);
  if (ret) {
    this->cubes[src_hmcId]->get_partial_link_graph(dst_hmcId)->links |= (0x1 << src_linkId);
    this->cubes[dst_hmcId]->get_partial_link_graph(src_hmcId)->links |= (0x1 << dst_linkId);
    this->cubes[src_hmcId]->hmc_routing_tables_update(); // just one needed ...
    this->cubes[src_hmcId]->hmc_routing_tables_visualize();

    this->link_garbage.push_back(link);
    return true;
  }
  else{
    delete[] link;
    return false;
  }
}

hmc_link*hmc_sim::hmc_link_to_slid(unsigned slidId, unsigned hmcId, unsigned linkId, enum link_width_t bitwidth)
{
  hmc_quad *quad = this->cubes[hmcId]->get_quad(linkId);

  hmc_link *link = new hmc_link[2];
  link[0].connect_linkports(&link[1]);
  link[1].re_adjust_links(bitwidth, 1);
  link[1].set_ilink_notify(slidId, this->slidnotify[slidId]); // important 1!! -> will be return for slid

  // notify all!
  for (unsigned i = 0; i < this->cubes.size(); i++)
    this->cubes[i]->set_slid(slidId, hmcId, linkId);

  if (quad->set_ext_link(&link[0])) {
    this->link_garbage.push_back(link);
    return &link[1];
  }
  else{
    delete[] link;
    return nullptr;
  }
}

void hmc_sim::clock(void)
{
  this->clk++;
  uint32_t notifymap = this->cubes_notify.get_notification();
  if (notifymap) {
    unsigned lid = __builtin_ctzl(notifymap);
    for (unsigned h = lid; h < this->cubes.size(); h++) {
      if ((0x1 << h) & notifymap) {
        this->cubes[h]->clock();
      }
    }
  }
}
