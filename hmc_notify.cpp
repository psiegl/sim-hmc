#include "hmc_notify.h"

hmc_notify::hmc_notify(unsigned id, hmc_notify *up, hmc_notify_cl* cl,
                       bool (hmc_notify_cl::*notify_up)(void)) :
  id(id),
  notifier(0),
  name(name),
  up(up),
  cl(cl),
  notify_up(notify_up)
{

}

hmc_notify::~hmc_notify(void)
{

}

void hmc_notify::notify_add(unsigned down_id)
{
  if( ! (this->notifier & (0x1ull << down_id)) )
  {
    this->notifier |= (0x1ull << down_id);
    if( up != nullptr )
    {
      up->notify_add(this->id);
    }
  }
}

#include <iostream>
void hmc_notify::notify_del(unsigned down_id)
{
  this->notifier &= ~(0x1ull << down_id);
  if( up != nullptr && (this->cl->*notify_up)() )
  {
    up->notify_del(this->id);
  }
}

uint32_t hmc_notify::get_notification(void)
{
  return this->notifier;
}
