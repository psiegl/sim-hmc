#ifndef _HMC_LINK_H_
#define _HMC_LINK_H_

#include <stdint.h>
#include "hmc_link_buf.h"
#include "hmc_link_queue.h"
#include "hmc_notify.h"

class hmc_link : private hmc_notify_cl {
  hmc_notify not_rx;
  hmc_link_queue rx;

  hmc_notify not_rx_buf;
  hmc_link_buf rx_buf;

  hmc_link_queue *tx;

  hmc_link *binding;

public:
  hmc_link(uint64_t *i_cur_cycle);
  virtual ~hmc_link(void);

  hmc_link_buf* get_ibuf(void);
  hmc_link_queue* __get_ilink(void); // ToDo: rx
  hmc_link_queue* get_olink(void); // ToDo: tx

  void set_ilink_notify(unsigned id, hmc_notify *notify);

  void re_adjust_links(unsigned link_bitwidth, float link_bitrate);
  void re_adjust_size(unsigned buf_bitsize);

  // setup of two parts of hmc_link to form ONE link
  void connect_linkports(hmc_link *part);
  void set_binding(hmc_link* part);

  void clock(void);
  bool notify_up(void);
};

#endif /* #ifndef _HMC_LINK_H_ */
