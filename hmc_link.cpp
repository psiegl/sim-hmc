#include "hmc_link.h"

hmc_link::hmc_link(void) :
  o( nullptr ),
  binding( nullptr )
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

void hmc_link::re_adjust_links(unsigned bitwidth, unsigned queuedepth)
{
  this->i.re_adjust(bitwidth, queuedepth);
  this->get_ilink()->re_adjust(bitwidth, queuedepth);
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
