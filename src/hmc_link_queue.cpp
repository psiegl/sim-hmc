#include <cassert>
#include "hmc_link_buf.h"
#include "hmc_link_queue.h"
#include "hmc_notify.h"

// everything related to occupation will be in Mega instead of Giga!
// therewith we can use safer unsigned, instead of float
hmc_link_queue::hmc_link_queue(uint64_t *cur_cycle, hmc_link_buf *buf, hmc_notify *notify) :
  id(-1),
  cur_cycle(cur_cycle),
  bitoccupation(0),
  bitoccupationmax(0),
  bitwidth(0),
  bitrate(0),
  notify(notify),
  buf(buf)
{
}

hmc_link_queue::~hmc_link_queue(void)
{
}

void hmc_link_queue::set_id(unsigned id)
{
  this->id = id;
}

void hmc_link_queue::re_adjust(unsigned link_bitwidth, float link_bitrate)
{
  this->bitrate = link_bitrate;
  this->bitwidth = link_bitwidth;
  this->bitoccupationmax = link_bitrate * link_bitwidth;
}

bool hmc_link_queue::has_space(unsigned packetleninbit)
{
  assert(this->bitoccupationmax); // otherwise not initialized!
  return ((this->bitoccupation / 1000) < this->bitoccupationmax);
}

bool hmc_link_queue::push_back(char *packet, unsigned packetleninbit)
{
  if ((this->bitoccupation / 1000) /* + packetleninbit */ < this->bitoccupationmax) {
    if (!this->bitoccupation)
      this->notify->notify_add(this->id);

    this->bitoccupation += ((packetleninbit / this->bitwidth) * 1000);
    float UI = packetleninbit / this->bitwidth;
    this->list.push_back(std::make_tuple(packet, UI, packetleninbit, *this->cur_cycle));
    return true;
  }
  return false;
}

void hmc_link_queue::clock(void)
{
  assert(!this->list.empty());
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

  auto front = this->list.front();
  if (std::get<1>(front) == 0.0f) {
    this->buf->push_back_set_avail(std::get<0>(front), std::get<2>(front));
    this->list.pop_front();
    if (!this->list.size())
      this->notify->notify_del(this->id);
  }
}
