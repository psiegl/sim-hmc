#include <cassert>
#include "hmc_link_buf.h"
#include "hmc_notify.h"
#include "hmc_link.h"
#ifdef HMC_LOGGING
# include "hmc_module.h"
# include "hmc_packet.h"
# include "hmc_decode.h"
# include "hmc_trace.h"
#endif /* #ifdef HMC_LOGGING */


hmc_link_buf::hmc_link_buf(uint64_t *cur_cycle, hmc_notify *notify, hmc_link *link) :
  cur_cycle(cur_cycle),
  bitoccupation(0.f),
  bitoccupationmax(0),
  notify(notify),
  link(link)
{
}

hmc_link_buf::~hmc_link_buf(void)
{
}

void hmc_link_buf::adjust_size(unsigned bitsize)
{
  this->bitoccupationmax = bitsize;
}

bool hmc_link_buf::reserve_space(float packetleninbit)
{
  assert(packetleninbit <= this->bitoccupationmax);
  if ((this->bitoccupation + packetleninbit) <= this->bitoccupationmax) {
    this->bitoccupation += packetleninbit;
#ifdef HMC_USES_NOTIFY
    if (!this->buf.size())
      this->notify->notify_add(0);
#endif /* #ifdef HMC_USES_NOTIFY */
    return true;
  }
  return false;
}

void hmc_link_buf::push_back_set_avail(char *packet, unsigned packetleninbit)
{
  this->buf.push_back(std::make_pair(packet, packetleninbit));
}

char* hmc_link_buf::front(unsigned *packetleninbit)
{
  if (this->buf.size()) {
    auto front = this->buf.front();
    *packetleninbit = front.second;
    return front.first;
  }
  return nullptr;
}

void hmc_link_buf::pop_front(void)
{
  switch (this->buf.size()) {
  case 1:
#ifdef HMC_USES_NOTIFY
    this->notify->notify_del(0);
#endif /* #ifdef HMC_USES_NOTIFY */
  // no break!!
  //-> in the case there is only one, we turn off notify,
  // because afterwards there is nothing left
  default:
  {
#ifdef HMC_LOGGING
    auto front = this->buf.front();
    char *packet = front.first;
    hmc_module *fromModule = this->link->get_binding()->get_module();
    int fromId = (fromModule) ? fromModule->get_id() : -1;
    hmc_module *toModule = this->link->get_module();
    int toId = (toModule) ? toModule->get_id() : -1;

    uint64_t header = HMC_PACKET_HEADER(packet);
    if (HMCSIM_PACKET_IS_REQUEST(header)) {
      uint64_t tail = HMC_PACKET_REQ_TAIL(packet);
      hmc_trace::trace_out_rqst(*cur_cycle, (uint64_t)packet, this->link->get_type(), fromId, toId, header, tail);
    }
    else {
      uint64_t tail = HMC_PACKET_RESP_TAIL(packet);
      hmc_trace::trace_out_rsp(*cur_cycle, (uint64_t)packet, this->link->get_type(), fromId, toId, header, tail);
    }
#endif /* #ifdef HMC_LOGGING */
    this->bitoccupation -= this->buf.front().second;
    this->buf.pop_front();
  }
  }
}
