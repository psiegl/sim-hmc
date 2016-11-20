#ifndef _HMC_SIM_H_
#define _HMC_SIM_H_

#include <cstdint>
#include <map>
#include <list>
#include <zlib.h> // crc32(), uLong
#include "config.h"
#include "hmc_jtag.h"
#include "hmc_macros.h"
#include "hmc_notify.h"
#include "hmc_queue.h"
#include "hmc_vault.h"

class hmc_link;
class hmc_cube;

typedef enum{
  RD_RS     = 0x38,
  WR_RS     = 0x39,
  MD_RD_RS  = 0x3A,
  MD_WR_RS  = 0x3B,
  RSP_ERROR = 0x3E,
  RSP_NONE  = 0x00,	// not really defined, but let's take it
  RSP_CMC
} hmc_response_t;

class hmc_sim : private hmc_notify_cl {
private:
  uint64_t clk;
  hmc_notify cubes_notify;
  std::map<unsigned, hmc_cube*> cubes;
  hmc_jtag* jtags[HMC_MAX_DEVS];

  std::map<unsigned, hmc_link*> slids;
  std::map<unsigned, hmc_notify*> slidnotify;

  std::list<hmc_link*> link_garbage;

  bool notify_up(void);

  unsigned seq = 0x0;
  ALWAYS_INLINE uint8_t hmcsim_rqst_getseq(hmc_rqst_t cmd) // ToDo!
  {
    if ((cmd == PRET) || (cmd == IRTRY))
      return seq;

    seq++;
    if (seq > 0x07)
      seq = 0x00;

    return 0x01;
  }
  ALWAYS_INLINE uint8_t hmcsim_rqst_getrrp(void)
  {
    return 0x03;
  }
  ALWAYS_INLINE uint8_t hmcsim_rqst_getfrp(void)
  {
    return 0x02;
  }
  ALWAYS_INLINE uint8_t hmcsim_rqst_getrtc(void)
  {
    return 0x01;
  }
  ALWAYS_INLINE uint32_t hmcsim_crc32(unsigned char *packet, unsigned flits)
  {

    uLong crc = crc32(0L, Z_NULL, 0);
    unsigned len = (flits << 1) * sizeof(uint64_t);
    /*
       As incoming packets flow through the link slave CRC is calculated from the header to
       the tail (inserting 0s into the CRC field) of every packet.
     */
    return crc32(crc, packet, len);
  }

public:
  hmc_sim(unsigned num_hmcs, unsigned num_slids,
          unsigned num_links, unsigned capacity,
          enum link_width_t ringbuswidth,
          enum link_width_t vaultbuswidth);
  ~hmc_sim(void);

  ALWAYS_INLINE hmc_jtag* hmc_get_jtag_interface(unsigned id)
  {
    return this->jtags[id];
  }
  bool hmc_set_link_config(unsigned src_hmcId, unsigned src_linkId,
                           unsigned dst_hmcId, unsigned dst_linkId,
                           enum link_width_t bitwidth);
  hmc_notify* hmc_define_slid(unsigned slidId, unsigned hmcId,
                            unsigned linkId, enum link_width_t bitwidth);

  bool hmc_send_pkt(unsigned slidId, void *pkt);
  bool hmc_recv_pkt(unsigned slidId, void *pkt);

  void hmc_decode_pkt(void *packet, uint64_t *header, uint64_t *tail,
                      hmc_response_t *type, unsigned *flits, uint16_t *tag,
                      uint8_t *slid, uint8_t *rrp, uint8_t *frp, uint8_t *seq,
                      uint8_t *dinv, uint8_t *errstat, uint8_t *rtc, uint32_t *crc);
  void hmc_encode_pkt(unsigned cub, unsigned slid, uint64_t addr,
                      uint16_t tag, hmc_rqst_t cmd, void *packet);

  void clock(void);
};

#endif /* #ifndef _HMC_SIM_H_ */
