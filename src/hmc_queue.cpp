#include <cassert>
#include "hmc_queue.h"

hmc_queue::hmc_queue(void) :
  id(-1),
  notify(NULL),
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

void hmc_queue::re_adjust(enum link_width_t linkwidth, unsigned queuedepth)
{
  this->linkwidth = linkwidth;
  this->bitoccupationmax = (this->linkwidth*8) * queuedepth;
}

bool hmc_queue::has_space(unsigned packetleninbit)
{
  assert(this->bitoccupationmax); // otherwise not initialized!
  return(this->bitoccupation /* + packetleninbit */ < this->bitoccupationmax);
}

int hmc_queue::push_back(void *packet, unsigned packetleninbit)
{
  if (this->bitoccupation /* + packetleninbit */ < this->bitoccupationmax) {
    if (this->notify != NULL && !this->bitoccupation)
      this->notify->notify_add(this->id);

    this->bitoccupation += packetleninbit;
    unsigned cycles = packetleninbit / (this->linkwidth*8);
    if (!cycles)
      cycles = 1;
    this->list.push_back(std::make_tuple(packet, cycles, packetleninbit));
    return 0;
  }
  return -1;
}

void*hmc_queue::front(unsigned *packetleninbit)
{
  assert(!this->list.empty());
  auto *q = &this->list;
  // ToDo: shall be all decreased or just the first ones?
  for (auto it = q->begin(); it != q->end(); ++it) {
    unsigned cyclestowait = std::get<1>(*it);
    if (cyclestowait > 0)
      cyclestowait = --std::get<1>(q->front());
  }
  unsigned cyclestowait = std::get<1>(q->front());
  *packetleninbit = std::get<2>(q->front());
  return (!cyclestowait) ? std::get<0>(q->front()) : nullptr;
}

void*hmc_queue::pop_front(void)
{
  std::list< std::tuple<void*, unsigned, unsigned> > *q = &this->list;
  void *front = std::get<0>(q->front());
  this->bitoccupation -= std::get<2>(q->front());
  q->pop_front();
  if (this->notify != NULL && !this->bitoccupation) {
    this->notify->notify_del(this->id);
  }
  return front;
}
