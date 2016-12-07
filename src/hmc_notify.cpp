#include "hmc_notify.h"

hmc_notify::hmc_notify(void) :
  id(0),
  notifier(0),
  up(nullptr),
  notify(nullptr)
{
}

hmc_notify::hmc_notify(unsigned id, hmc_notify *up, hmc_notify_cl *notify) :
  id(id),
  notifier(0),
  up(up),
  notify(notify)
{
}

hmc_notify::~hmc_notify(void)
{
}

void hmc_notify::set(unsigned id, hmc_notify *up)
{
  this->id = id;
  this->up = up;
}

void hmc_notify::notify_add(unsigned down_id)
{
  if (!(this->notifier & (0x1 << down_id))) {
    this->notifier |= (0x1 << down_id);
    if (this->up != nullptr)
      this->up->notify_add(this->id);
  }
}

void hmc_notify::notify_del(unsigned down_id)
{
  this->notifier &= ~(0x1 << down_id);
  if (this->up != nullptr && this->notify->notify_up())
    this->up->notify_del(this->id);
}
