#include <cassert>
#include "hmc_queue.h"
#include "hmc_notify.h"

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
  this->bitoccupationmax = (this->linkwidth * 8) * queuedepth;
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
    if (!cycles)
      cycles = 1;
    this->list.push_back(std::make_tuple(packet, cycles, packetleninbit));
    return true;
  }
  return false;
}

char* hmc_queue::front(unsigned *packetleninbit)
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

char* hmc_queue::pop_front(void)
{
  std::list< std::tuple<char*, unsigned, unsigned> > *q = &this->list;
  char *front = std::get<0>(q->front());
  this->bitoccupation -= std::get<2>(q->front());
  q->pop_front();
  if (this->notify != NULL && !this->bitoccupation) {
    this->notify->notify_del(this->id);
  }
  return front;
}
