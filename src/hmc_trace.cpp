#include "hmc_trace.h"

static hmc_trace_logger *logger = nullptr;
hmc_trace_logger **hmc_trace_log = &logger;

void hmc_trace::trace_in_rqst(uint64_t cycle, uint64_t phyPktAddr,
                              unsigned typeId, int fromId, int toId,
                              uint64_t header, uint64_t tail)
{
  logger->execute(typeId, 0x0, cycle, phyPktAddr, fromId, toId, header, tail);
}

void hmc_trace::trace_in_rsp(uint64_t cycle, uint64_t phyPktAddr,
                             unsigned typeId, int fromId, int toId,
                             uint64_t header, uint64_t tail)
{
  logger->execute(typeId, 0x1, cycle, phyPktAddr, fromId, toId, header, tail);
}

void hmc_trace::trace_out_rqst(uint64_t cycle, uint64_t phyPktAddr,
                               unsigned typeId, int fromId, int toId,
                               uint64_t header, uint64_t tail)
{
  logger->execute(typeId, 0x2, cycle, phyPktAddr, fromId, toId, header, tail);
}

void hmc_trace::trace_out_rsp(uint64_t cycle, uint64_t phyPktAddr,
                              unsigned typeId, int fromId, int toId,
                              uint64_t header, uint64_t tail)
{
  logger->execute(typeId, 0x3, cycle, phyPktAddr, fromId, toId, header, tail);
}
