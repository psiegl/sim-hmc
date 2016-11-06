#include <cassert>
#include "hmc_queue.h"

hmc_queue::hmc_queue(void) :
  id( -1 ),
  notify( NULL ),
  bitoccupation( 0 ),
  bitoccupationmax( 0 ),
  bitwidth( 0 )
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

void hmc_queue::re_adjust(unsigned bitwidth, unsigned queuedepth)
{
  this->bitwidth = bitwidth;
  this->bitoccupationmax = bitwidth * queuedepth;
}

int hmc_queue::has_space(unsigned packetleninbit)
{
  return ! (this->bitoccupation /* + packetleninbit */ < this->bitoccupationmax);
}

int hmc_queue::push_back(void *packet, unsigned packetleninbit)
{
  if( this->bitoccupation /* + packetleninbit */ < this->bitoccupationmax ) {
    this->bitoccupation += packetleninbit;
    unsigned cycles = packetleninbit / this->bitwidth;
    if( ! cycles )
      cycles = 1;
    this->queue.push(std::make_tuple(packet, cycles, packetleninbit));
    if( this->notify != NULL )
      this->notify->notify_add(this->id);
    return 0;
  }
  return -1;
}

void* hmc_queue::front(void)
{
  assert( ! this->queue.empty());
  std::queue< std::tuple<void*, unsigned, unsigned> > * q = &this->queue;
  unsigned cyclestowait = std::get<1>(q->front());
  if(cyclestowait > 0)
    cyclestowait = --std::get<1>(q->front());
  return ( ! cyclestowait) ? std::get<0>(q->front()) : nullptr;
}

void* hmc_queue::pop_front(void)
{
  void* front = std::get<0>(this->queue.front());
  this->queue.pop();
  if( this->notify != NULL )
    this->notify->notify_del(this->id);
  return front;
}
