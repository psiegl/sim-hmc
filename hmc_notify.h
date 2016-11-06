#ifndef _HMC_NOTIFY_H_
#define _HMC_NOTIFY_H_

#include <cstdint>

class hmc_notify {

  // current
  unsigned id;
  uint32_t notifier;

  // upwards
  hmc_notify* up;

public:
  hmc_notify(unsigned id, hmc_notify *up);
  ~hmc_notify(void);

  void notify_add(unsigned id);
  void notify_del(unsigned id);

  uint32_t get_notification(void);
};

#endif /* #ifndef _HMC_NOTIFY_H_ */
