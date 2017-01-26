#ifndef _HMC_VAULT_H_
#define _HMC_VAULT_H_

#include <cstdint>
#include <tuple>
#if defined(NDEBUG) && defined(HMC_USES_CRC)
# include <zlib.h>
#endif /* #if defined(NDEBUG) && defined(HMC_USES_CRC) */
#ifdef HMC_USES_BOBSIM
# include <cassert>
#endif /* #ifdef HMC_USES_BOBSIM */
#include "hmc_macros.h"
#include "hmc_notify.h"
#include "hmc_sim_t.h"
#include "hmc_link.h"
#include "hmc_module.h"

class hmc_cube;

struct jtl_t {
  hmc_rqst_t rsqt;
  unsigned rsp_len;
  hmc_response_t rsp_cmd;
  bool rsp;
};

class hmc_vault : public hmc_module, private hmc_notify_cl {
private:
  unsigned id;
  hmc_link *link;
  hmc_notify link_notify;
  hmc_cube *cube;

  ALWAYS_INLINE uint32_t hmcsim_crc32(void *packet, unsigned flits)
  {
#if defined(NDEBUG) && defined(HMC_USES_CRC)
    uLong crc = crc32(0L, Z_NULL, 0);
    unsigned len = (flits << 1) * sizeof(uint64_t);
    /*
       As incoming packets flow through the link slave CRC is calculated from the header to
       the tail (inserting 0s into the CRC field) of every packet.
     */
    return crc32(crc, (const Bytef *)packet, len);
#else
    return 0;
#endif /* #if defined(NDEBUG) && defined(HMC_USES_CRC) */
  }

  struct jtl_t* jtl[ 0xFF ];
  struct jtl_t jtli[58] = {
    { WR16, 1, WR_RS, true },
    { WR32, 1, WR_RS, true },
    { WR48, 1, WR_RS, true },
    { WR64, 1, WR_RS, true },
    { WR80, 1, WR_RS, true },
    { WR96, 1, WR_RS, true },
    { WR112, 1, WR_RS, true },
    { WR128, 1, WR_RS, true },
    { WR256, 1, WR_RS, true },
    { MD_WR, 1, MD_WR_RS, true },
    { BWR, 1, WR_RS, true },
    { TWOADD8, 0, WR_RS, true },
    { ADD16, 0, WR_RS, true },
    { P_WR16, 0, RSP_ERROR, false },
    { P_WR32, 0, RSP_ERROR, false },
    { P_WR48, 0, RSP_ERROR, false },
    { P_WR64, 0, RSP_ERROR, false },
    { P_WR80, 0, RSP_ERROR, false },
    { P_WR96, 0, RSP_ERROR, false },
    { P_WR112, 0, RSP_ERROR, false },
    { P_WR128, 0, RSP_ERROR, false },
    { P_WR256, 0, RSP_ERROR, false },
    { P_BWR, 0, RSP_ERROR, false },
    { P_2ADD8, 0, RSP_ERROR, false },
    { P_ADD16, 0, RSP_ERROR, false },
    { RD16, 2, RD_RS, true },
    { RD32, 3, RD_RS, true },
    { RD48, 4, RD_RS, true },
    { RD64, 5, RD_RS, true },
    { RD80, 6, RD_RS, true },
    { RD96, 7, RD_RS, true },
    { RD112, 8, RD_RS, true },
    { RD128, 9, RD_RS, true },
    { RD256, 17, RD_RS, true },
    { MD_RD, 2, MD_RD_RS, true },
    { FLOW_NULL, 0, RSP_ERROR, false },
    { PRET, 0, RSP_ERROR, false },
    { TRET, 0, RSP_ERROR, false },
    { IRTRY, 0, RSP_ERROR, false },
    { TWOADDS8R, 2, RD_RS, true },
    { ADDS16R, 2, RD_RS, true },
    { INC8, 1, WR_RS, true },
    { P_INC8, 0, RSP_ERROR, false },
    { XOR16, 2, RD_RS, false },
    { OR16, 2, RD_RS, false },
    { NOR16, 2, RD_RS, false },
    { AND16, 2, RD_RS, false },
    { NAND16, 2, RD_RS, false },
    { CASGT8, 2, RD_RS, false },
    { CASGT16, 2, RD_RS, false },
    { CASLT8, 2, RD_RS, false },
    { CASLT16, 2, RD_RS, false },
    { CASEQ8, 2, RD_RS, false },
    { CASZERO16, 2, RD_RS, false },
    { EQ8, 1, WR_RS, true },
    { EQ16, 1, WR_RS, true },
    { BWR8R, 2, RD_RS, true },
    { SWAP16, 2, RD_RS, true }
  };

  bool notify_up(void) {
#ifdef HMC_USES_NOTIFY
    return (!this->link_notify.get_notification());
#else
    return true;
#endif /* #ifdef HMC_USES_NOTIFY */
  }

public:
  hmc_vault(unsigned id, hmc_cube *cube, hmc_notify* notify);
  ~hmc_vault(void);
#ifdef HMC_USES_BOBSIM
  void clock(void) {}
#else
  void clock(void);
#endif /* #ifdef HMC_USES_BOBSIM */
  unsigned get_id(void) { return this->id; }
  bool set_link(unsigned linkId, hmc_link* link, enum hmc_link_type linkType) {
    this->link = link;
    this->link->set_ilink_notify(linkId, linkId, &this->link_notify);
    return true;
  }
  bool hmcsim_process_rqst(void *packet);
  bool hmcsim_packet_resp_len(hmc_rqst_t cmd, unsigned *rsp_len);
#ifdef HMC_USES_BOBSIM
#ifdef HMC_USES_NOTIFY
  bool pkt_has_response(hmc_rqst_t cmd)
  {
    struct jtl_t* cmd_type = this->jtl[cmd];
    assert(cmd_type != nullptr);
    return cmd_type->rsp;
  }
#endif /* #ifdef HMC_USES_NOTIFY */
#endif /* #ifdef HMC_USES_BOBSIM */
};

#endif /* #ifndef _HMC_VAULT_H_ */
