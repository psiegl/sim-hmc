#include <cassert>
#include "hmc_link_queue.h"
#include "hmc_notify.h"

// everything related to occupation will be in Mega instead of Giga!
// therewith we can use safer unsigned, instead of float
hmc_link_queue::hmc_link_queue(uint64_t *cur_cycle, hmc_link_buf *buf, hmc_notify *notify) :
  id(-1),
  cur_cycle(cur_cycle),
  bitoccupation(0),
  bitoccupationmax(30.0f * HMCSIM_QUARTER_LINK_WIDTH),
  bitwidth(HMCSIM_QUARTER_LINK_WIDTH),
  bitrate(30.0f),
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
  this->bitwidth = link_bitwidth;
  this->bitoccupationmax = bitrate /*ToDo*/ * link_bitwidth;
}

bool hmc_link_queue::has_space(unsigned packetleninbit)
{
  assert(this->bitoccupationmax); // otherwise not initialized!
  return ((this->bitoccupation / 1000) < this->bitoccupationmax);
  //{
  //bool tmp = this->buf->reserve_space(packetleninbit);
  //std::cout << tmp << std::endl;
  //  return true; // ToDo
  //}
  //return false;
}

bool hmc_link_queue::push_back(char *packet, unsigned packetleninbit)
{
  if ((this->bitoccupation / 1000) /* + packetleninbit */ < this->bitoccupationmax) {
    if (!this->bitoccupation) {
      this->notify->notify_add(this->id);
    }

    this->bitoccupation += (packetleninbit / this->bitwidth * 1000);
    float UI = packetleninbit / this->bitwidth;
    this->list.push_back(std::make_tuple(packet, UI, packetleninbit, *this->cur_cycle));
    return true;
  }
  return false;
}

char* hmc_link_queue::front(unsigned *packetleninbit)
{
  assert(!this->list.empty());
  float tbitrate = this->bitrate;
  for (auto it = this->list.begin(); it != this->list.end(); ++it) {
    float UI = std::get<1>(*it);
    if (UI > tbitrate) {
      std::get<1>(*it) -= tbitrate;
      this->bitoccupation -= (unsigned)(tbitrate * 1000);
      break;
    }
    else {
      tbitrate -= UI;
      this->bitoccupation -= (unsigned)(std::get<1>(*it) * 1000);
      std::get<1>(*it) = 0.0f;
    }
  }
  auto front = this->list.front();
  *packetleninbit = std::get<2>(front);
  return (!((unsigned)std::get<1>(front))
          && std::get<3>(front) != *this->cur_cycle) ? std::get<0>(front) : nullptr;
}

void hmc_link_queue::pop_front(void)
{
  this->list.pop_front();
  if (!this->list.size()) {
    this->notify->notify_del(this->id);
  }
}

void hmc_link_queue::clock(void)
{
  assert(!this->list.empty());
  float tbitrate = this->bitrate;
  for (auto it = this->list.begin(); it != this->list.end(); ++it) {
    float UI = std::get<1>(*it);
    if (UI > tbitrate) {
      std::get<1>(*it) -= tbitrate;
      this->bitoccupation -= (unsigned)(tbitrate * 1000);
      break;
    }
    else {
      tbitrate -= UI;
      this->bitoccupation -= (unsigned)(std::get<1>(*it) * 1000);
      std::get<1>(*it) = 0.0f;
    }
  }

  auto front = this->list.front();
  if (!((unsigned)std::get<1>(front))
      && std::get<3>(front) != *this->cur_cycle) {
    if (this->buf->reserve_space(std::get<2>(front))) { // could be also above, but then it needs to add bitrate per clk to the buffer
      this->buf->push_back_set_avail(std::get<0>(front), std::get<2>(front));
      this->list.pop_front();
      if (!this->list.size()) {
        this->notify->notify_del(this->id);
      }
    }
  }
}

