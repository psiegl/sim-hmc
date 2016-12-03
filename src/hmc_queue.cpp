#include <cassert>
#include "hmc_queue.h"
#include "hmc_notify.h"

hmc_queue::hmc_queue(uint64_t *cur_cycle) :
  id(-1),
  notify(NULL),
  cur_cycle(cur_cycle),
  bitoccupation(0),
  bitoccupationmax(0),
  linkwidth(HMCSIM_QUARTER_LINK_WIDTH)
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

void hmc_queue::re_adjust(enum link_width_t lanes, unsigned queuedepth)
{
  this->linkwidth = lanes;
  this->bitoccupationmax = (lanes * 8) * queuedepth;
}

bool hmc_queue::has_space(unsigned packetleninbit)
{
  assert(this->bitoccupationmax); // otherwise not initialized!
  return (this->bitoccupation /* + packetleninbit */ < this->bitoccupationmax);
}

bool hmc_queue::push_back(char *packet, unsigned packetleninbit)
{
  if (this->bitoccupation /* + packetleninbit */ < this->bitoccupationmax) {
    if (this->notify != NULL && !this->bitoccupation)
      this->notify->notify_add(this->id);

    this->bitoccupation += packetleninbit;
    unsigned cycles = packetleninbit / (this->linkwidth * 8);
    this->list.push_back(std::make_tuple(packet, cycles, packetleninbit, *this->cur_cycle));
    return true;
  }
  return false;
}

char* hmc_queue::front(unsigned *packetleninbit)
{
  assert(!this->list.empty());
  // ToDo: shall be all decreased or just the first ones?
  for (auto it = this->list.begin(); it != this->list.end(); ++it) {
    std::get<1>(*it) -= (std::get<1>(*it) > 0);
  }
  auto front = this->list.front();
  unsigned cyclestowait = std::get<1>(front);
  *packetleninbit = std::get<2>(front);
  return (!cyclestowait && std::get<3>(front) != *this->cur_cycle) ? std::get<0>(front) : nullptr;
}

char* hmc_queue::pop_front(void)
{
  char *front = std::get<0>(this->list.front());
  this->bitoccupation -= std::get<2>(this->list.front());
  this->list.pop_front();
  if (this->notify != NULL && !this->bitoccupation) {
    this->notify->notify_del(this->id);
  }
  return front;
}
