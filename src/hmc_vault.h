#ifndef _HMC_VAULT_H_
#define _HMC_VAULT_H_

#include <cstdint>
#include <tuple>
#ifndef NDEBUG
#include <zlib.h>
#endif
#include "hmc_macros.h"
#include "hmc_sim_t.h"

class hmc_link;
class hmc_cube;
class hmc_notify;

struct jtl_t {
  hmc_rqst_t rsqt;
  unsigned rsp_len;
  hmc_response_t rsp_cmd;
  bool rsp;
};

class hmc_vault {
private:
  hmc_link *link;
  hmc_cube *cube;

  ALWAYS_INLINE uint32_t hmcsim_crc32(void *packet, unsigned flits)
  {
#ifndef NDEBUG
    uLong crc = crc32(0L, Z_NULL, 0);
    unsigned len = (flits << 1) * sizeof(uint64_t);
    /*
       As incoming packets flow through the link slave CRC is calculated from the header to
       the tail (inserting 0s into the CRC field) of every packet.
     */
    return crc32(crc, (const Bytef *)packet, len);
#else
    return 0;
#endif
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

protected:
  bool hmcsim_process_rqst(void *packet);
  bool hmcsim_packet_resp_len(hmc_rqst_t cmd, unsigned *rsp_len);

public:
  hmc_vault(unsigned id, hmc_cube *cube, hmc_notify* notify, hmc_link *link);
  ~hmc_vault(void);
#ifndef HMC_USES_BOBSIM
  void clock(void);
#endif /* #ifndef HMC_USES_BOBSIM */
};

#endif /* #ifndef _HMC_VAULT_H_ */
