#include "hmc_notify.h"

hmc_notify::hmc_notify(unsigned id, hmc_notify *up, hmc_notify_cl* notify) :
  id(id),
  notifier(0),
  up(up),
  notify(notify)
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

void hmc_notify::notify_del(unsigned down_id)
{
  this->notifier &= ~(0x1ull << down_id);
  if( up != nullptr && this->notify->notify_up() )
  {
    up->notify_del(this->id);
  }
}

uint32_t hmc_notify::get_notification(void)
{
  return this->notifier;
}
