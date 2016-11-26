#ifndef _HMC_DECODE_H_
#define _HMC_DECODE_H_

#include <cstdint>
#include <math.h>
#include "config.h"
#include "hmc_macros.h"

/* ----------------------------------------- REQUEST / RESPONSE specified in the HEADER of the packet .. unofficial */

// [23:23] in both packet headers reserved ... unofficial helper
#define HMCSIM_PACKET_IS_REQUEST(P)             (!(((P) >> 23) & 0x1))
#define HMCSIM_PACKET_IS_RESPONSE(P)            ((((P) >> 23) & 0x1))

#define HMCSIM_PACKET_SET_REQUEST()             (0x0)
#define HMCSIM_PACKET_SET_RESPONSE()            (0x1 << 23)


/* ----------------------------------------- REQUEST specified in the HEADER of the packet */

/* -- command = [6:0] */
#define HMCSIM_PACKET_REQUEST_GET_CMD(P)        ((P) & 0x7F)
#define HMCSIM_PACKET_REQUEST_SET_CMD(P)        ((P) & 0x7F)

/* -- packet length in flits = [11:7] */
#define HMCSIM_PACKET_REQUEST_GET_LNG(P)        (((P) >> 7) & 0x1F)
#define HMCSIM_PACKET_REQUEST_SET_LNG(P)        (((P) & 0x1F) << 7)

/* -- tag = [22:12] */
#define HMCSIM_PACKET_REQUEST_GET_TAG(P)        (((P) >> 12) & 0x7FF)
#define HMCSIM_PACKET_REQUEST_SET_TAG(P)        (((P) & 0x7FF) << 12)

/* -- address = [57:24] */
#define HMCSIM_PACKET_REQUEST_GET_ADRS(P)       (((P) >> 24) & 0x3FFFFFFFFull)
#define HMCSIM_PACKET_REQUEST_SET_ADRS(P)       (((uint64_t)((P) & 0x3FFFFFFFFull)) << 24)

/* -- cube id = [63:61] */
#define HMCSIM_PACKET_REQUEST_GET_CUB(P)        (((P) >> 61) & 0x7)
#define HMCSIM_PACKET_REQUEST_SET_CUB(P)        (((uint64_t)((P) & 0x7)) << 61)


/* ----------------------------------------- REQUEST specified in the TAIL of the packet */

/* -- return retry pointer = [8:0] */
#define HMCSIM_PACKET_REQUEST_GET_RRP(P)        ((P) & 0x1FF)
#define HMCSIM_PACKET_REQUEST_SET_RRP(P)        ((P) & 0x1FF)

/* -- forward retry pointer = [17:9] */
#define HMCSIM_PACKET_REQUEST_GET_FRP(P)        (((P) >> 9) & 0x1FF)
#define HMCSIM_PACKET_REQUEST_SET_FRP(P)        (((P) & 0x1FF) << 9)

/* -- sequence number = [20:18] */
#define HMCSIM_PACKET_REQUEST_GET_SEQ(P)        (((P) >> 18) & 0x7)
#define HMCSIM_PACKET_REQUEST_SET_SEQ(P)        (((P) & 0x7) << 18)

/* -- poison bit = [21:21] */
#define HMCSIM_PACKET_REQUEST_GET_Pb(P)         (((P) >> 21) & 0x1)
#define HMCSIM_PACKET_REQUEST_SET_Pb(P)         (((P) & 0x1) << 21)

/* -- source link id = [28:26] */
#define HMCSIM_PACKET_REQUEST_GET_SLID(P)       (((P) >> 26) & 0x7)
#define HMCSIM_PACKET_REQUEST_SET_SLID(P)       (((P) & 0x7) << 26)

/* -- return token count = [31:29] */
#define HMCSIM_PACKET_REQUEST_GET_RTC(P)        (((P) >> 29) & 0x7)
#define HMCSIM_PACKET_REQUEST_SET_RTC(P)        (((P) & 0x7) << 29)

/* -- cyclic redundancy check = [63:32] */
#define HMCSIM_PACKET_REQUEST_GET_CRC(P)        (((P) >> 32) & 0xFFFFFFFFull)
#define HMCSIM_PACKET_REQUEST_SET_CRC(P)        (((uint64_t)((P) & 0xFFFFFFFFull)) << 32)



/* ----------------------------------------- RESPONSE specified in the HEADER of the packet */

/* -- command = [6:0] */
#define HMCSIM_PACKET_RESPONSE_GET_CMD(P)       ((P) & 0x7F)
#define HMCSIM_PACKET_RESPONSE_SET_CMD(P)       ((P) & 0x7F)

/* -- packet length in flits = [11:7] */
#define HMCSIM_PACKET_RESPONSE_GET_LNG(P)       (((P) >> 7) & 0x1F)
#define HMCSIM_PACKET_RESPONSE_SET_LNG(P)       (((P) & 0x1F) << 7)

/* -- tag = [22:12] */
#define HMCSIM_PACKET_RESPONSE_GET_TAG(P)       (((P) >> 12) & 0x7FF)
#define HMCSIM_PACKET_RESPONSE_SET_TAG(P)       (((P) & 0x7FF) << 12)

/* -- atomic flag = [33:33] */
#define HMCSIM_PACKET_RESPONSE_GET_AF(P)        (((P) >> 33) & 0x1)
#define HMCSIM_PACKET_RESPONSE_SET_AF(P)        (((uint64_t)((P) & 0x1)) << 33)

/* -- source link id = [41:39] */
#define HMCSIM_PACKET_RESPONSE_GET_SLID(P)      (((P) >> 39) & 0x7)
#define HMCSIM_PACKET_RESPONSE_SET_SLID(P)      (((uint64_t)((P) & 0x7)) << 39)

/* -- cube id = [63:61] */
#define HMCSIM_PACKET_RESPONSE_GET_CUB(P)       (((P) >> 61) & 0x7)
#define HMCSIM_PACKET_RESPONSE_SET_CUB(P)       (((uint64_t)((P) & 0x7)) << 61)


/* ----------------------------------------- RESPONSE specified in the TAIL of the packet */

/* -- return retry pointer = [8:0] */
#define HMCSIM_PACKET_RESPONSE_GET_RRP(P)       ((P) & 0x1FF)
#define HMCSIM_PACKET_RESPONSE_SET_RRP(P)       ((P) & 0x1FF)

/* -- forward retry pointer = [17:9] */
#define HMCSIM_PACKET_RESPONSE_GET_FRP(P)       (((P) >> 9) & 0x1FF)
#define HMCSIM_PACKET_RESPONSE_SET_FRP(P)       (((P) & 0x1FF) << 9)

/* -- sequence number = [20:18] */
#define HMCSIM_PACKET_RESPONSE_GET_SEQ(P)       (((P) >> 18) & 0x7)
#define HMCSIM_PACKET_RESPONSE_SET_SEQ(P)       (((P) & 0x7) << 18)

/* -- data invalid = [21:21] */
#define HMCSIM_PACKET_RESPONSE_GET_DINV(P)      (((P) >> 21) & 0x1)
#define HMCSIM_PACKET_RESPONSE_SET_DINV(P)      (((P) & 0x1) << 21)

/* -- error status = [28:22] */
#define HMCSIM_PACKET_RESPONSE_GET_ERRSTAT(P)   (((P) >> 22) & 0x7F)
#define HMCSIM_PACKET_RESPONSE_SET_ERRSTAT(P)   (((P) & 0x7F) << 22)

/* -- return token counts = [31:29] */
#define HMCSIM_PACKET_RESPONSE_GET_RTC(P)       (((P) >> 29) & 0x7)
#define HMCSIM_PACKET_RESPONSE_SET_RTC(P)       (((P) & 0x7) << 29)

/* -- cyclic redundancy check = [63:32] */
#define HMCSIM_PACKET_RESPONSE_GET_CRC(P)       (((P) >> 32) & 0xFFFFFFFFull)
#define HMCSIM_PACKET_RESPONSE_SET_CRC(P)       (((uint64_t)((P) & 0xFFFFFFFFull)) << 32)

/* ----------------------------------------- */

#define HMC_PACKET_HEADER( x )    (((uint64_t*)(x))[0])
#define HMC_PACKET_REQ_TAIL( x )  (((uint64_t*)(x))[(HMCSIM_PACKET_REQUEST_GET_LNG(HMC_PACKET_HEADER(x)) << 1)-1])

/* ----------------------------------------- */

class hmc_decode {
  unsigned vault_shift;
  uint32_t vault_mask;

  unsigned quad_shift;
  uint32_t quad_mask;

  unsigned bank_shift;
  uint32_t bank_mask;

  unsigned dram_shift_lo;
  uint32_t dram_mask_lo;

  unsigned dram_shift_hi;
  uint32_t dram_mask_hi;

  unsigned row_shift;
  uint32_t row_mask;

  unsigned col_shift;
  uint32_t col_mask;

public:
  hmc_decode(unsigned bsize, unsigned num_banks_per_vault);
  ~hmc_decode(void);
  void set_decoding(unsigned bsize, unsigned num_banks_per_vault);


  ALWAYS_INLINE unsigned HMCSIM_UTIL_DECODE_QUAD( uint64_t addr )
  {
    return (addr >> this->quad_shift) & this->quad_mask;
  }
//  ALWAYS_INLINE uint64_t HMCSIM_UTIL_ENCODE_QUAD( unsigned quad )
//  {
//    return (uint64_t)(quad & this->quad_mask) << this->quad_shift;
//  }

  ALWAYS_INLINE unsigned HMCSIM_UTIL_DECODE_VAULT( uint64_t addr )
  {
    return (addr >> this->vault_shift) & this->vault_mask;
  }
//  ALWAYS_INLINE uint64_t HMCSIM_UTIL_ENCODE_VAULT( unsigned vault )
//  {
//    return (uint64_t)(vault & this->vault_mask) << this->vault_shift;
//  }

  ALWAYS_INLINE unsigned HMCSIM_UTIL_DECODE_BANK( uint64_t addr )
  {
    return (addr >> this->bank_shift) & this->bank_mask;
  }
//  ALWAYS_INLINE uint64_t HMCSIM_UTIL_ENCODE_BANK( unsigned bank )
//  {
//    return (uint64_t)(bank & this->bank_mask) << this->bank_shift;
//  }

  ALWAYS_INLINE uint64_t HMC_UTIL_DECODE_DRAM( uint64_t addr )
  {
    uint64_t hi = (addr >> this->dram_shift_hi) & this->dram_mask_hi;
    uint64_t lo = (addr >> this->dram_shift_lo) & this->dram_mask_lo;
    return (uint64_t)(hi | lo);
  }

  ALWAYS_INLINE void HMC_UTIL_DECODE_COL_AND_ROW(uint64_t addr, unsigned *col, unsigned *row)
  {
    uint64_t dram_addr = this->HMC_UTIL_DECODE_DRAM(addr);
    *col = (dram_addr >> this->col_shift) & this->col_mask;
    *row = (dram_addr >> this->row_shift) & this->row_mask;
  }
};

#endif /* #ifndef _HMC_DECODE_H_ */
