#include <cassert>
#include "hmc_queue.h"
#include "hmc_notify.h"

// everything related to occupation will be in Mega instead of Giga!
// therewith we can use safer unsigned, instead of float
hmc_queue::hmc_queue(uint64_t *cur_cycle) :
  id(-1),
  notify(NULL),
  cur_cycle(cur_cycle),
  bitoccupation(0),
  bitoccupationmax(30.0f * HMCSIM_QUARTER_LINK_WIDTH),
  bitwidth(HMCSIM_QUARTER_LINK_WIDTH),
  bitrate(30.0f)
{
}

hmc_queue::~hmc_queue(void)
{
}

void hmc_queue::set_notify(unsigned id, hmc_notify *notify)
{
  this->id = id;
  this->notify = notify;
}

void hmc_queue::re_adjust(unsigned link_bitwidth, float link_bitrate)
{
  this->bitwidth = link_bitwidth;
  this->bitoccupationmax = bitrate /*ToDo*/ * link_bitwidth;
}

bool hmc_queue::has_space(unsigned packetleninbit)
{
  assert(this->bitoccupationmax); // otherwise not initialized!
  return ((this->bitoccupation / 1000) < this->bitoccupationmax);
}

bool hmc_queue::push_back(char *packet, unsigned packetleninbit)
{
  if ((this->bitoccupation / 1000) /* + packetleninbit */ < this->bitoccupationmax) {
    if (this->notify != NULL && !this->bitoccupation)
      this->notify->notify_add(this->id);

    this->bitoccupation += (packetleninbit / this->bitwidth * 1000);
    float UI = packetleninbit / this->bitwidth;
    this->list.push_back(std::make_tuple(packet, UI, packetleninbit, *this->cur_cycle));
    return true;
  }
  return false;
}

char* hmc_queue::front(unsigned *packetleninbit)
{
  assert(!this->list.empty());
  // ToDo: shall be all decreased or just the first ones?
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
  return (!((unsigned)std::get<1>(front)) && std::get<3>(front) != *this->cur_cycle) ? std::get<0>(front) : nullptr;
}

char* hmc_queue::pop_front(void)
{
  char *front = std::get<0>(this->list.front());
  this->list.pop_front();
  if (this->notify != NULL && !this->list.size()) {
    this->notify->notify_del(this->id);
  }
  return front;
}
