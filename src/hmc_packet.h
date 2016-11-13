#ifndef _HMC_PACKET_H_
#define _HMC_PACKET_H_

#include <cstdint>
//#include "../include/hmc_sim_macros.h"

/* ----------------------------------------- REQUEST / RESPONSE specified in the HEADER of the packet .. unofficial */

// [23:23] in both packet headers reserved ... unofficial helper
#define HMCSIM_PACKET_IS_REQUEST(P)             (!(((P) >> 23) & 0x1))
#define HMCSIM_PACKET_IS_RESPONSE(P)            ((((P) >> 23) & 0x1))

#define HMCSIM_PACKET_SET_REQUEST()             (0x0)
#define HMCSIM_PACKET_SET_RESPONSE()            (0x1 << 23)

#ifdef HMC_HAS_LOGIC

/* specified in the TAIL of the packet */
/* [22:22] specify if from logic or host */
#define HMCSIM_PACKET_REQUEST_SET_FROM_LOGIC()  (0x1 << 22)
#define HMCSIM_PACKET_REQUEST_GET_FROM_LOGIC(P) (((P) >> 22) & 0x1)

/* [28:23] specify (if from logic, the defined addr of logic) */
#define HMCSIM_PACKET_REQUEST_SET_LOGIC_ADDR(P) (((P) & 0x3F) << 23)
#define HMCSIM_PACKET_REQUEST_GET_LOGIC_ADDR(P) (((P) >> 23) & 0x3F)

#define HMCSIM_PACKET_RESPONSE_SET_TO_LOGIC()   (0x1ull << 34)
#define HMCSIM_PACKET_RESPONSE_IS_TO_LOGIC(P)   (((P) >> 34) & 0x1)

#define HMCSIM_PACKET_RESPONSE_SET_LOGIC_ADDR(P)((((uint64_t)(P)) & 0x3F) << 35)
#define HMCSIM_PACKET_RESPONSE_GET_LOGIC_ADDR(P)(((P) >> 35) & 0x3F)

#define HMCSIM_LOGIC_GET_CUB(P)                 ((P) & 0x7)
#define HMCSIM_LOGIC_SET_CUB(P)                 ((P) & 0x7)
#define HMCSIM_LOGIC_GET_QUAD(P)                (((P) >> 3) & 0x3)
#define HMCSIM_LOGIC_SET_QUAD(P)                (((P) & 0x3) << 3)
#define HMCSIM_LOGIC_GET_LOGICADDR(P)           (((P) >> 5) & 0x1)
#define HMCSIM_LOGIC_SET_LOGICADDR(P)           (((P) & 0x1) << 5)
#endif /* #ifdef HMC_HAS_LOGIC */


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


#endif /* #ifndef _HMC_PACKET_H_ */
