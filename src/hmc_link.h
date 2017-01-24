#ifndef _HMC_LINK_H_
#define _HMC_LINK_H_

#include <cstdint>
#include "hmc_link_fifo.h"
#include "hmc_link_queue.h"
#include "hmc_notify.h"
#include "hmc_macros.h"

class hmc_module;
class hmc_cube;

enum hmc_link_type {
  HMC_LINK_EXTERN    = 0x0,
  HMC_LINK_RING      = 0x1,
  HMC_LINK_VAULT_IN  = 0x2,
  HMC_LINK_VAULT_OUT = 0x3,
  HMC_LINK_SLID      = 0x4
};

class hmc_link : private hmc_notify_cl {
private:
  hmc_module *module;
  enum hmc_link_type type;
  hmc_notify not_rx_q;
  hmc_link_queue rx_q;

  hmc_notify not_rx_buf;
  hmc_link_fifo rx_fifo_out;

#ifdef HMC_LOGGING
  hmc_cube *cube;
#endif /* #ifdef HMC_LOGGING */

  // to bind the other end ...
  hmc_link_queue *tx;
  hmc_link *binding;


public:
  hmc_link(uint64_t *i_cur_cycle, enum hmc_link_type type,
           hmc_module *module, hmc_cube *cube = nullptr,
           unsigned linkId_in_module = ~0x0);
  virtual ~hmc_link(void);

  // ToDo: tx buf!
  ALWAYS_INLINE hmc_link_fifo* get_rx_fifo_out(void)
  {
    return &this->rx_fifo_out;
  }

  hmc_link_queue* __get_rx_q(void);
  ALWAYS_INLINE hmc_link_queue* get_tx(void)
  {
    return this->tx;
  }
  ALWAYS_INLINE hmc_module* get_module(void)
  {
    return this->module;
  }
  ALWAYS_INLINE hmc_link* get_binding(void)
  {
    return this->binding;
  }
  ALWAYS_INLINE enum hmc_link_type get_type(void)
  {
    return this->type;
  }
#ifdef HMC_LOGGING
  ALWAYS_INLINE hmc_cube* get_cube(void)
  {
    return this->cube;
  }
#endif /* #ifdef HMC_LOGGING */

  void set_ilink_notify(unsigned notifyid, unsigned id, hmc_notify *notify);

  void adjust_both_linkends(unsigned link_bitwidth, float link_bitrate, unsigned link_fifo_out_sizeInFlits);

  // setup of two parts of hmc_link to form ONE link
  void connect_linkports(hmc_link *part);
  void set_binding(hmc_link* part);

  void clock(void);
  bool notify_up(void);
};

#endif /* #ifndef _HMC_LINK_H_ */
