#ifndef _HMC_SIM_H_
#define _HMC_SIM_H_

#include <cstdint>
#include <map>
#include <list>
#if defined(NDEBUG) && defined(HMC_USES_CRC)
#include <zlib.h> // crc32(), uLong
#endif /* #if defined(NDEBUG) && defined(HMC_USES_CRC) */
#include "config.h"
#include "hmc_sim_t.h"
#include "hmc_jtag.h"
#include "hmc_macros.h"
#include "hmc_notify.h"

/* link bit rate in Gb/s */
#define HMCSIM_BR12_5   12.5f
#define HMCSIM_BR15     15.0f
#define HMCSIM_BR25     25.0f
#define HMCSIM_BR28     28.0f
#define HMCSIM_BR30     30.0f /* 30Gb/s -> 15 GHz (DDR) -> 0.06666ns (48x 312.5MHz) */

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
};

class hmc_link;
class hmc_cube;
class hmc_slid;

class hmc_sim : private hmc_notify_cl {
private:
  uint64_t clk;
  hmc_notify cubes_notify;
  std::map<unsigned, hmc_cube*> cubes;
  hmc_jtag* jtags[HMC_MAX_DEVS];

  std::map<unsigned, hmc_link*> slids;

  hmc_notify slidnotify;
  hmc_notify slidbufnotify;
  unsigned num_slids;
  unsigned num_links;

  std::list<hmc_link*> link_garbage;
  std::list<hmc_slid*> slidModule_garbage;

  bool notify_up(unsigned id);

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
#if defined(NDEBUG) && defined(HMC_USES_CRC)
    uLong crc = crc32(0L, Z_NULL, 0);
    unsigned len = (flits << 1) * sizeof(uint64_t);
    /*
       As incoming packets flow through the link slave CRC is calculated from the header to
       the tail (inserting 0s into the CRC field) of every packet.
     */
    return crc32(crc, packet, len);
#else
    return 0;
#endif /* #if defined(NDEBUG) && defined(HMC_USES_CRC) */
  }

public:
  hmc_sim(unsigned num_cubes, unsigned num_slids,
          unsigned num_links, unsigned capacity,
          unsigned quadbus_bitwidth, float quadbus_bitrate);
  ~hmc_sim(void);

  ALWAYS_INLINE hmc_jtag* hmc_get_jtag_interface(unsigned cubeId)
  {
    return this->jtags[cubeId];
  }
  bool hmc_set_link_config(unsigned src_cubeId, unsigned src_linkId,
                           unsigned dst_cubeId, unsigned dst_linkId,
                           unsigned bitwidth, float bitrate);
  hmc_notify* hmc_define_slid(unsigned slidId, unsigned cubeId,
                              unsigned linkId,
                              unsigned lanes, float bitrate);
#ifdef HMC_USES_GRAPHVIZ
  hmc_notify* hmc_get_slid_notify(void);
#endif /* #ifdef HMC_USES_GRAPHVIZ */

  bool hmc_send_pkt(unsigned slidId, char *pkt);
  bool hmc_recv_pkt(unsigned slidId, char *pkt);

  void hmc_decode_pkt(char *packet, uint64_t *header, uint64_t *tail,
                      hmc_response_t *type, unsigned *flits, uint16_t *tag,
                      uint8_t *slid, uint8_t *rrp, uint8_t *frp, uint8_t *seq,
                      uint8_t *dinv, uint8_t *errstat, uint8_t *rtc, uint32_t *crc);
  void hmc_encode_pkt(unsigned cub, unsigned quad, unsigned vault, unsigned bank, unsigned dram,
                      uint16_t tag, hmc_rqst_t cmd, char *packet);
  void hmc_encode_pkt(unsigned cub, uint64_t addr,
                      uint16_t tag, hmc_rqst_t cmd, char *packet);

  void clock(void);
  uint64_t hmc_get_clock(void) {
    return this->clk;
  }
};

#endif /* #ifndef _HMC_SIM_H_ */
