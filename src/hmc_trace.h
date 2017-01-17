#ifndef _HMC_TRACE_H_
#define _HMC_TRACE_H_

#include <stdint.h>

class hmc_trace_logger {
public:
  virtual ~hmc_trace_logger(void) {}
  virtual void execute(unsigned linkTypeId, unsigned linkIntTypeId,
                       uint64_t cycle, uint64_t phyPktAddr,
                       int fromId, int toId,
                       uint64_t header, uint64_t tail) = 0;
};

class hmc_trace {
public:
  static void trace_in_rqst(uint64_t cycle, uint64_t phyPktAddr,
                            unsigned typeId, int fromId, int toId,
                            uint64_t header, uint64_t tail);
  static void trace_in_rsp(uint64_t cycle, uint64_t phyPktAddr,
                           unsigned typeId, int fromId, int toId,
                           uint64_t header, uint64_t tail);
  static void trace_out_rqst(uint64_t cycle, uint64_t phyPktAddr,
                             unsigned typeId, int fromId, int toId,
                             uint64_t header, uint64_t tail);
  static void trace_out_rsp(uint64_t cycle, uint64_t phyPktAddr,
                            unsigned typeId, int fromId, int toId,
                            uint64_t header, uint64_t tail);
};

#endif /* #ifndef _HMC_TRACE_H_ */