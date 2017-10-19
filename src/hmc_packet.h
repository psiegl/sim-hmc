#ifndef _HMC_PACKET_H_
#define _HMC_PACKET_H_

#include <cstdint>
//#include "../include/hmc_sim_macros.h"

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


#endif /* #ifndef _HMC_PACKET_H_ */
