#include <iostream>
#include "config.h"
#include "hmc_link.h"
#include "hmc_module.h"

hmc_link::hmc_link(uint64_t *i_cur_cycle, enum hmc_link_type type,
                   hmc_module *module, hmc_cube *cube,
                   unsigned linkId_in_module) :
  hmc_notify_cl(),
  module(module),
  type(type),
  not_rx_q(-1, nullptr, this),
  rx_q(i_cur_cycle, &rx_fifo_out, &not_rx_q, this),
  not_rx_buf(-1, nullptr, this),
  rx_fifo_out(i_cur_cycle, &not_rx_buf, this),
#ifdef HMC_LOGGING
  cube(cube),
#endif /* #ifdef HMC_LOGGING */
  tx(nullptr),
  binding(nullptr)
{
  if (this->module == nullptr) {
    std::cerr << "ERROR: can't register link to module!" << std::endl;
    exit(-1);
  }
  this->module->set_link(linkId_in_module, this, type); // ToDo: if false!
}

hmc_link::~hmc_link(void)
{
}

hmc_link_queue* hmc_link::__get_rx_q(void)
{
  return &this->rx_q;
}

void hmc_link::set_ilink_notify(unsigned notifyid, unsigned id, hmc_notify *notify)
{
  this->not_rx_q.set(notifyid, notify);
  this->not_rx_buf.set(notifyid, notify);

  this->rx_q.set_notifyid(notifyid, id);
}


void hmc_link::adjust_both_linkends(unsigned link_bitwidth, float link_bitrate, unsigned link_fifo_out_sizeInFlits)
{
  this->__get_rx_q()->re_adjust(link_bitwidth, link_bitrate);
  this->binding->__get_rx_q()->re_adjust(link_bitwidth, link_bitrate);

  this->get_rx_fifo_out()->adjust_size(link_fifo_out_sizeInFlits);
  this->binding->get_rx_fifo_out()->adjust_size(link_fifo_out_sizeInFlits);
}

void hmc_link::connect_linkports(hmc_link *part)
{
  this->set_binding(part);
  part->set_binding(this);
}

void hmc_link::set_binding(hmc_link *part)
{
  this->tx = part->__get_rx_q();
  this->binding = part;
}


void hmc_link::clock(void)
{
#ifdef HMC_USES_NOTIFY
  if (this->not_rx_q.get_notification())
#endif /* #ifdef HMC_USES_NOTIFY */
  {
    this->rx_q.clock();
  }
// ToDo -> will be most likely a combination out of front and pop_front
}

bool hmc_link::notify_up(void)
{
#ifdef HMC_USES_NOTIFY
  return (!this->not_rx_q.get_notification()
          && !this->not_rx_buf.get_notification());
#else
  return true;
#endif /* #ifdef HMC_USES_NOTIFY */
}
