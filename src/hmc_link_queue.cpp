#include <cassert>
#include "hmc_link.h"
#include "hmc_link_buf.h"
#include "hmc_link_queue.h"
#include "hmc_notify.h"
#include "config.h"
#include "hmc_module.h"
#ifdef HMC_LOGGING
# include "hmc_packet.h"
# include "hmc_decode.h"
# include "hmc_trace.h"
#endif /* #ifdef HMC_LOGGING */

// everything related to occupation will be in Mega instead of Giga!
// therewith we can use safer unsigned, instead of float
hmc_link_queue::hmc_link_queue(uint64_t *cur_cycle, hmc_link_buf *buf,
                               hmc_notify *notify, hmc_link *link) :
  id(-1),
  notifyid(-1),
  cur_cycle(cur_cycle),
  bitoccupation(0),
  bitoccupationmax(0),
  bitwidth(0),
  bitrate(0),
  notify(notify),
  buf(buf),
  link(link),
  link_end(nullptr)
{
}

hmc_link_queue::~hmc_link_queue(void)
{
}

void hmc_link_queue::set_notifyid(unsigned notifyid, unsigned id)
{
  this->notifyid = notifyid;
  this->id = id;
}

// we run here relative to 1 GHz, just to have a possiblity to be in sync with BOBSIM
void hmc_link_queue::re_adjust(unsigned link_bitwidth, float link_bitrate)
{
  this->bitrate = link_bitrate / (1.0f / (float)HMC_CLK_PERIOD_NS);
  this->bitwidth = link_bitwidth;
  this->bitoccupationmax = this->bitrate * link_bitwidth;
}

bool hmc_link_queue::has_space(unsigned packetleninbit)
{
  assert(this->bitoccupationmax); // otherwise not initialized!
  return ((this->bitoccupation / 1000) < this->bitoccupationmax);
}

//#define HMC_LINK_QUEUE_PERFORMANCE__UNSAFE 1
/*
 * in theory, has_space will always be executed checked before push_back will be executed
 * If full performance is required, check first if else case below will be entered .. if not ... go for it :-)
 */
bool hmc_link_queue::push_back(char *packet, unsigned packetleninbit)
{
#ifndef HMC_LINK_QUEUE_PERFORMANCE__UNSAFE
  if (__builtin_expect((this->bitoccupation / 1000) /* + packetleninbit */ < this->bitoccupationmax, 1))
#endif /* #ifndef HMC_LINK_QUEUE_PERFORMANCE__UNSAFE */
  {
#ifdef HMC_USES_NOTIFY
    if (!this->bitoccupation)
      this->notify->notify_add(this->notifyid);
#endif /* #ifdef HMC_USES_NOTIFY */

    this->bitoccupation += ((packetleninbit / this->bitwidth) * 1000);
    float UI = packetleninbit / this->bitwidth;
    this->list.push_back(std::make_tuple(packet, UI, packetleninbit, *this->cur_cycle));
#ifdef HMC_LOGGING
    hmc_module *fromModule = this->link->get_binding()->get_module();
    int fromId = (fromModule) ? fromModule->get_id() : -1;
    hmc_module *toModule = this->link->get_module();
    int toId = (toModule) ? toModule->get_id() : -1;

    uint64_t header = HMC_PACKET_HEADER(packet);
    if (HMCSIM_PACKET_IS_REQUEST(header)) {
      uint64_t tail = HMC_PACKET_REQ_TAIL(packet);
      hmc_trace::trace_in_rqst(*cur_cycle, (uint64_t)packet, this->link->get_type(), fromId, toId, header, tail);
    }
    else {
      uint64_t tail = HMC_PACKET_RESP_TAIL(packet);
      hmc_trace::trace_in_rsp(*cur_cycle, (uint64_t)packet, this->link->get_type(), fromId, toId, header, tail);
    }
#endif /* #ifdef HMC_LOGGING */
    return true;
  }

  // shouldn't happen
#ifndef HMC_LINK_QUEUE_PERFORMANCE__UNSAFE
  return false;
#endif /* #ifndef HMC_LINK_QUEUE_PERFORMANCE__UNSAFE */
}

void hmc_link_queue::clock(void)
{
#ifdef HMC_USES_NOTIFY
  assert(!this->list.empty());
#endif /* #ifdef HMC_USES_NOTIFY */
  if (this->bitoccupation) { // speedup, it could be that clock is issued, but there is no bitoccupation, but still elements left, since it could not yet fit into the buffer
    float tbitrate = this->bitrate;
    for (auto it = this->list.begin(); it != this->list.end(); ++it) {
      if (std::get<3>(*it) == *this->cur_cycle)
        break;

      float UI = std::get<1>(*it);
      if (UI == 0.0f)
        continue;

      if (UI > tbitrate) {
        if (this->buf->reserve_space(tbitrate)) {
          std::get<1>(*it) -= tbitrate;
          this->bitoccupation -= (unsigned)(tbitrate * 1000);
        }
        break;
      }
      else if (this->buf->reserve_space(tbitrate)) {
        std::get<1>(*it) = 0.0f;
        this->buf->reserve_space(UI);
        this->bitoccupation -= (unsigned)(UI * 1000);
        tbitrate -= UI;
      }
      else
        break;
    }
  }

#ifndef HMC_USES_NOTIFY
  if (this->list.empty())
    return;
#endif /* #ifndef HMC_USES_NOTIFY */
  auto front = this->list.front();
  if (std::get<1>(front) == 0.0f) {
    char *packet = std::get<0>(front);
#ifdef HMC_LOGGING
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
    this->buf->push_back_set_avail(packet, std::get<2>(front));
    this->list.pop_front();
#ifdef HMC_USES_NOTIFY
    if (!this->list.size())
      this->notify->notify_del(this->notifyid);
#endif /* #ifdef HMC_USES_NOTIFY */
  }
}
