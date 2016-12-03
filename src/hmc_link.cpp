#include "hmc_link.h"
#include "hmc_notify.h"

hmc_link::hmc_link(uint64_t *i_cur_cycle) :
  i(i_cur_cycle),
  o(nullptr),
  binding(nullptr)
{
}

hmc_link::~hmc_link(void)
{
}

hmc_queue* hmc_link::get_ilink(void)
{
  return &this->i;
}

hmc_queue* hmc_link::get_olink(void)
{
  return this->o;
}

void hmc_link::set_ilink_notify(unsigned id, hmc_notify *notify)
{
  this->i.set_notify(id, notify);
}

void hmc_link::re_adjust_links(enum link_width_t lanes, unsigned queuedepth)
{
  this->get_ilink()->re_adjust(lanes, queuedepth);
  this->binding->get_ilink()->re_adjust(lanes, queuedepth);
}

void hmc_link::connect_linkports(hmc_link *part)
{
  this->set_binding(part);
  part->set_binding(this);
}

void hmc_link::set_binding(hmc_link *part)
{
  this->o = part->get_ilink();
  this->binding = part;
}
