#ifndef _HMC_LINK_QUEUE_H_
#define _HMC_LINK_QUEUE_H_

#include <list>
#include <stdint.h>
#include <tuple>
//#include <map>
#include "hmc_link_buf.h"
#include "hmc_notify.h"


/* lanes -> each lane 1 bit per unit interval (UI). 1 UI -> 100ps (is set by the ref. clock oscillator) HMCv1.0
Link serialization occurs with the least-significant portion of the FLIT traversing across
the lanes of the link first. During one unit interval (UI) a single bit is transferred across
each lane of the link. For the full-width configuration, 16 bits are transferred simultaneously
during the UI, so it takes 8 UIs to transfer the entire 128-bit FLIT. For the half-width
configuration, 8 bits are transferred simultaneously, taking 16 UIs to transfer a single
FLIT. The following table shows the relationship of the FLIT bit positions to the lanes
during each UI for both full-width and half-width configurations.
*/
enum link_width_t {
  HMCSIM_FULL_LINK_WIDTH    = 16,
  HMCSIM_HALF_LINK_WIDTH    =  8,
  HMCSIM_QUARTER_LINK_WIDTH =  4
}; // ToDo: move to hmc_sim.h

// tuple( packetptr, amount of cycles, totalsizeinbits );
class hmc_link_queue {
private:
  unsigned id;
  uint64_t *cur_cycle;

  unsigned bitoccupation;
  unsigned bitoccupationmax;

  unsigned bitwidth;
  float bitrate;

  hmc_notify *notify;
  std::list< std::tuple<char*, float, unsigned, uint64_t> > list;
  hmc_link_buf *buf;

//  std::map<char*, uint64_t> tmp;

public:
  hmc_link_queue(uint64_t* cur_cycle, hmc_link_buf *buf, hmc_notify *notify);
  ~hmc_link_queue(void);

  void set_id(unsigned id);

  void re_adjust(unsigned link_bitwidth, float link_bitrate);

  bool has_space(unsigned packetleninbit);
  bool push_back(char *packet, unsigned packetleninbit);

  void clock(void);
};

#endif /* #ifndef _HMC_LINK_QUEUE_H_ */
