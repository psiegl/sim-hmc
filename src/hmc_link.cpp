#include "hmc_link.h"
#include "hmc_notify.h"

hmc_link::hmc_link(uint64_t *i_cur_cycle) :
  rx(i_cur_cycle),
  tx(nullptr),
  binding(nullptr)
{
}

hmc_link::~hmc_link(void)
{
}

hmc_link_queue* hmc_link::get_ilink(void)
{
  return &this->rx;
}

hmc_link_queue* hmc_link::get_olink(void)
{
  return this->tx;
}

void hmc_link::set_ilink_notify(unsigned id, hmc_notify *notify)
{
  this->rx.set_notify(id, notify);
}

void hmc_link::re_adjust_links(unsigned link_bitwidth, float link_bitrate)
{
  this->get_ilink()->re_adjust(link_bitwidth, link_bitrate);
  this->binding->get_ilink()->re_adjust(link_bitwidth, link_bitrate);
}

void hmc_link::connect_linkports(hmc_link *part)
{
  this->set_binding(part);
  part->set_binding(this);
}

void hmc_link::set_binding(hmc_link *part)
{
  this->tx = part->get_ilink();
  this->binding = part;
}
