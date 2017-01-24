#ifndef _HMC_TRACE_STDOUT_H_
#define _HMC_TRACE_STDOUT_H_

#include <cstdint>
#include <iostream>
#include "hmc_trace.h"

class hmc_trace_stdout : public hmc_trace_logger {
public:
  hmc_trace_stdout(void) {}
  ~hmc_trace_stdout(void) {}

  void execute(enum hmc_link_type linkTypeId, unsigned linkIntTypeId,
               uint64_t cycle, uint64_t phyPktAddr,
               int fromCubId, int toCubId,
               int fromId, int toId,
               uint64_t header, uint64_t tail)
  {
    std::cout << "HMCSIM_LOG (" << cycle << ")";

    switch(linkIntTypeId) {
    case 0x0:
      std::cout << " in_rqst";
      break;
    case 0x1:
      std::cout << " in_rsp";
      break;
    case 0x2:
      std::cout << " out_rqst";
      break;
    case 0x3:
      std::cout << " out_rsp";
      break;
    }

    switch(linkTypeId) {
    case HMC_LINK_EXTERN: // link
      std::cout << ", hmc"<< fromCubId <<" (quad"<< fromId <<") to hmc"<< toCubId <<" (quad"<< toId <<")";
      break;
    case HMC_LINK_RING:
      std::cout << ", hmc"<< fromCubId <<", quad"<< fromId <<" to quad"<< toId; // fromCubID == toCubId
      break;
    case HMC_LINK_VAULT_IN:
      std::cout << ", hmc"<< fromCubId <<", quad"<< fromId <<" to vault"<< toId; // fromCubID == toCubId
      break;
    case HMC_LINK_VAULT_OUT:
      std::cout << ", hmc"<< fromCubId <<", vault"<< fromId <<" to quad"<< toId; // fromCubID == toCubId
      break;
    case HMC_LINK_SLID:
      if(fromCubId == -1)
        std::cout << ", slid"<< fromId <<" to hmc"<< toCubId <<" (quad"<< toId <<")";
      else
        std::cout << ", hmc"<< fromCubId <<" (quad"<< fromId <<") to slid"<< toId;
      break;
    }

    std::cout << ", header: " << std::hex << header;
    std::cout << ", tail: " << std::hex << tail;
    std::cout << ", phyPktAddr: " << std::hex << phyPktAddr;
    std::cout << std::dec << std::endl;
  }
};
#endif /* #ifndef _HMC_TRACE_STDOUT_H_ */
