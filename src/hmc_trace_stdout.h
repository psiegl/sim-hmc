#ifndef _HMC_TRACE_STDOUT_H_
#define _HMC_TRACE_STDOUT_H_

#include <cstdint>
#include <iostream>
#include "hmc_trace.h"

class hmc_trace_stdout : public hmc_trace_logger {
public:
  hmc_trace_stdout(void) {}
  ~hmc_trace_stdout(void) {}

  void execute(unsigned linkTypeId, unsigned linkIntTypeId,
               uint64_t cycle, uint64_t phyPktAddr,
               int fromId, int toId,
               uint64_t header, uint64_t tail)
  {
    std::cout << "HMCSIM_LOG (" << cycle << ")";
    std::cout << " linkTypeId: " << linkTypeId;
    std::cout << ", linkIntTypeId: " << linkIntTypeId;
    std::cout << ", phyPktAddr: " << phyPktAddr;
    std::cout << ", fromId: " << fromId;
    std::cout << ", toId: " << toId;
    std::cout << ", header: " << std::hex << header;
    std::cout << ", tail: " << std::hex << tail;
    std::cout << std::dec << std::endl;
  }
};
#endif /* #ifndef _HMC_TRACE_STDOUT_H_ */
