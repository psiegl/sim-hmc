#ifndef _HMC_NOTIFY_H_
#define _HMC_NOTIFY_H_

#include <cstdint>

class hmc_notify_cl {
public:
  hmc_notify_cl(void) {}
  ~hmc_notify_cl(void) {}

//  virtual bool clock(void) = 0;

  virtual bool notify_up(void) = 0;
};

class hmc_notify {

  // current
  unsigned id;
  uint32_t notifier;
  const char *name;

  // upwards
  hmc_notify* up;

  hmc_notify_cl* cl;
  bool (hmc_notify_cl::*notify_up)(void);

public:
  hmc_notify(unsigned id, hmc_notify *up, hmc_notify_cl* cl,
             bool (hmc_notify_cl::*notify_up)(void));
  ~hmc_notify(void);

  void notify_add(unsigned id);
  void notify_del(unsigned id);

  uint32_t get_notification(void);
};

#endif /* #ifndef _HMC_NOTIFY_H_ */
