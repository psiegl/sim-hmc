#include <cassert>
#include "hmc_link_fifo.h"
#include "hmc_notify.h"
#include "hmc_link.h"
#ifdef HMC_LOGGING
# include "hmc_module.h"
# include "hmc_packet.h"
# include "hmc_decode.h"
# include "hmc_trace.h"
# include "hmc_cube.h"
#endif /* #ifdef HMC_LOGGING */


hmc_link_fifo::hmc_link_fifo(uint64_t *cur_cycle, hmc_notify *notify, hmc_link *link) :
#ifdef HMC_LOGGING
  cur_cycle(cur_cycle),
  link(link),
#endif /* #ifdef HMC_LOGGING */
  bitoccupation(0),
  bitoccupationmax(0)
#ifdef HMC_USES_NOTIFY
  , notify(notify)
#endif /* #ifdef HMC_USES_NOTIFY */
{
}

hmc_link_fifo::~hmc_link_fifo(void)
{
}

void hmc_link_fifo::adjust_size(unsigned bitsize)
{
  this->bitoccupationmax = bitsize * 1000;
}

bool hmc_link_fifo::reserve_space(unsigned packetleninbit)
{
  assert(packetleninbit <= this->bitoccupationmax);
  if ((this->bitoccupation + packetleninbit) <= this->bitoccupationmax) {
    this->bitoccupation += packetleninbit;
    return true;
  }
  return false;
}

void hmc_link_fifo::push_back_set_avail(char *packet, unsigned packetleninbit)
{
#ifdef HMC_USES_NOTIFY
  if (!this->buf.size())
    this->notify->notify_add(0);
#endif /* #ifdef HMC_USES_NOTIFY */
  this->buf.push_back(std::make_pair(packet, packetleninbit));
}

char* hmc_link_fifo::front(unsigned *packetleninbit)
{
#ifndef HMC_USES_NOTIFY
  if (this->buf.size())
#endif /* #ifndef HMC_USES_NOTIFY */
  {
    auto front = this->buf.front();
    *packetleninbit = front.second / 1000;
    return front.first;
  }
#ifndef HMC_USES_NOTIFY
  return nullptr;
#endif /* #ifndef HMC_USES_NOTIFY */
}

void hmc_link_fifo::pop_front(void)
{
  switch (this->buf.size()) {
  case 0:
    break;
  case 1:
#ifdef HMC_USES_NOTIFY
    this->notify->notify_del(0);
#endif /* #ifdef HMC_USES_NOTIFY */
  // no break!!
  //-> in the case there is only one, we turn off notify,
  // because afterwards there is nothing left
  default:
  {
    auto front = this->buf.front();
#ifdef HMC_LOGGING
    char *packet = front.first;
    int fromId = this->link->get_binding()->get_module()->get_id();
    int toId = this->link->get_module()->get_id();
    hmc_cube *toCub = this->link->get_cube();
    int toCubId = (!toCub) ? -1 : toCub->get_id();
    hmc_cube *fromCub = this->link->get_binding()->get_cube();
    int fromCubId = (!fromCub) ? -1 : fromCub->get_id();

    uint64_t header = HMC_PACKET_HEADER(packet);
    if (HMCSIM_PACKET_IS_REQUEST(header)) {
      uint64_t tail = HMC_PACKET_REQ_TAIL(packet);
      hmc_trace::trace_out_rqst(*cur_cycle, (uint64_t)packet, this->link->get_type(), fromCubId, toCubId, fromId, toId, header, tail);
    }
    else {
      uint64_t tail = HMC_PACKET_RESP_TAIL(packet);
      hmc_trace::trace_out_rsp(*cur_cycle, (uint64_t)packet, this->link->get_type(), fromCubId, toCubId, fromId, toId, header, tail);
    }
#endif /* #ifdef HMC_LOGGING */
    this->bitoccupation -= front.second;
    this->buf.pop_front();
  }
  }
}
