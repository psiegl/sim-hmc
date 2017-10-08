#ifndef _HMC_NOTIFY_H_
#define _HMC_NOTIFY_H_

#include "hmc_macros.h"
#include <iostream>

class hmc_notify_cl {
public:
  hmc_notify_cl(void) {}
  ~hmc_notify_cl(void) {}

  virtual void clock(void) = 0;
  virtual bool notify_up(unsigned id) = 0;
};

class hmc_notify {
private:
  // current
  unsigned id;
  unsigned notifier;

  // upwards
  hmc_notify* up;
  hmc_notify_cl* notify;
  unsigned ncase;

public:
  hmc_notify(void) :
    id(0),
    notifier(0),
    up(nullptr),
    notify(nullptr),
    ncase(0)
  {}

  hmc_notify(unsigned id, hmc_notify *up, hmc_notify_cl *notify) :
    id(id),
    notifier(0),
    up(up),
    notify(notify),
    ncase(0)
  {}

  ~hmc_notify(void)
  {}

  ALWAYS_INLINE void set(unsigned id, hmc_notify *up, unsigned ncase = 0)
  {
    this->id = id;
    this->up = up;
    if(ncase)
      this->ncase = ncase;
  }

  void notify_add(unsigned down_id)
  {
    if (!(this->notifier & (0x1 << down_id))) {
      this->notifier |= (0x1 << down_id);
      if (this->up != nullptr)
        this->up->notify_add(this->id);
    }
  }

  void notify_del(unsigned down_id)
  {
    this->notifier &= ~(0x1 << down_id);
    if (this->up != nullptr && this->notify->notify_up(this->ncase))
      this->up->notify_del(this->id);
  }

  ALWAYS_INLINE unsigned get_notification(void)
  {
#ifdef HMC_USES_NOTIFY
     return this->notifier;
#else
    return ~0x0;
#endif /* #ifdef HMC_USES_NOTIFY */
  }
};

#endif /* #ifndef _HMC_NOTIFY_H_ */
