#include "hmc_link.h"

hmc_link::hmc_link(uint64_t *i_cur_cycle) :
  hmc_notify_cl(),
  not_rx(-1, nullptr, this),
  rx(i_cur_cycle, &rx_buf, &not_rx),
  not_rx_buf(-1, nullptr, this),
  rx_buf(&not_rx_buf),
  tx(nullptr),
  binding(nullptr)
{
}

hmc_link::~hmc_link(void)
{
}

hmc_link_buf* hmc_link::get_ibuf(void)
{
  return &this->rx_buf;
}

hmc_link_queue* hmc_link::__get_ilink(void)
{
  return &this->rx;
}

hmc_link_queue* hmc_link::get_olink(void)
{
  return this->tx;
}

void hmc_link::set_ilink_notify(unsigned id, hmc_notify *notify)
{
  this->not_rx.set(id, notify);
  this->not_rx_buf.set(id, notify);

  this->rx.set_id(id);
}


void hmc_link::re_adjust_links(unsigned link_bitwidth, float link_bitrate)
{
  this->__get_ilink()->re_adjust(link_bitwidth, link_bitrate);
  this->binding->__get_ilink()->re_adjust(link_bitwidth, link_bitrate);

  this->re_adjust_size(128 * 17);
  this->binding->re_adjust_size(128 * 17);
}

void hmc_link::re_adjust_size(unsigned buf_bitsize)
{
  this->rx_buf.adjust_size(buf_bitsize);
}

void hmc_link::connect_linkports(hmc_link *part)
{
  this->set_binding(part);
  part->set_binding(this);
}

void hmc_link::set_binding(hmc_link *part)
{
  this->tx = part->__get_ilink();
  this->binding = part;
}


void hmc_link::clock(void)
{
  if (this->not_rx.get_notification())
    this->rx.clock();
// ToDo -> will be most likely a combination out of front and pop_front
}

bool hmc_link::notify_up(void)
{
  return (!this->not_rx.get_notification()
          && !this->not_rx_buf.get_notification());
}
