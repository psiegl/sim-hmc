#ifndef _HMC_NOTIFY_H_
#define _HMC_NOTIFY_H_

#include "hmc_macros.h"

class hmc_notify_cl {
public:
  hmc_notify_cl(void) {}
  ~hmc_notify_cl(void) {}

  virtual void clock(void) = 0;
  virtual bool notify_up(void) = 0;
};

class hmc_notify {
private:
  // current
  unsigned id;
  unsigned notifier;

  // upwards
  hmc_notify* up;
  hmc_notify_cl* notify;

public:
  hmc_notify(void);
  hmc_notify(unsigned id, hmc_notify *up, hmc_notify_cl *notify);
  ~hmc_notify(void);
  void set(unsigned id, hmc_notify *up);

  void notify_add(unsigned id);
  void notify_del(unsigned id);

  ALWAYS_INLINE unsigned get_notification(void)
  {
     return this->notifier;
  }
};

#endif /* #ifndef _HMC_NOTIFY_H_ */
