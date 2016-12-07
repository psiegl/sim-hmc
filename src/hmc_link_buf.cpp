#include <cassert>
#include "hmc_link_buf.h"
#include "hmc_notify.h"

// ToDo: timestamp!

hmc_link_buf::hmc_link_buf(hmc_notify *notify) :
  bitoccupation(0.f),
  bitoccupationmax(0),
  notify(notify)
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
    if (!this->buf.size()) {
      this->notify->notify_add(0);
    }
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
    this->notify->notify_del(0);
  // no break!!
  //-> in the case there is only one, we turn off notify,
  // because afterwards there is nothing left
  default:
    this->bitoccupation -= this->buf.front().second;
    this->buf.pop_front();
  }
}
