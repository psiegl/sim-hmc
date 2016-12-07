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

    this->bitoccupation += ((packetleninbit / this->bitwidth) * 1000);
    float UI = packetleninbit / this->bitwidth;
    this->list.push_back(std::make_tuple(packet, UI, packetleninbit, *this->cur_cycle));
//    tmp[packet] = *this->cur_cycle;
    return true;
  }
  return false;
}

//#include <iostream>
void hmc_link_queue::clock(void)
{
  assert(!this->list.empty());
  float tbitrate = this->bitrate;
  for (auto it = this->list.begin(); it != this->list.end(); ++it) {
    if (std::get<3>(*it) == *this->cur_cycle) {
      break;
    }

    float UI = std::get<1>(*it);
    if (UI == 0.0f)
      continue;

    if (UI > tbitrate) {
      std::get<1>(*it) -= tbitrate;
      this->bitoccupation -= (unsigned)(tbitrate * 1000);
      break;
    }
    else {
      tbitrate -= UI;
      this->bitoccupation -= (unsigned)(UI * 1000);
      std::get<1>(*it) = 0.0f;
    }
  }

  auto front = this->list.front();
  if (std::get<1>(front) == 0.0f) {
    if (this->buf->reserve_space(std::get<2>(front))) { // could be also above, but then it needs to add bitrate per clk to the buffer
      this->buf->push_back_set_avail(std::get<0>(front), std::get<2>(front));

//      char *p = std::get<0>(front);
//      uint64_t in_cycle = tmp[p];
//      std::cout << "p: in: " << in_cycle << ", out: " << *this->cur_cycle;
//      std::cout << ", cycles: " << (*this->cur_cycle - in_cycle) << std::endl;

      this->list.pop_front();
      if (!this->list.size())
        this->notify->notify_del(this->id);
    }
  }
}

