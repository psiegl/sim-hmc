#ifndef _HMC_TRACE_HDF5_H_
#define _HMC_TRACE_HDF5_H_

#include <hdf5.h>
#include <stdint.h>
#include "hmc_trace.h"
#include "/usr/include/hdf5.h"

class hmc_hdf5 : public hmc_trace_logger {
private:


public:
  explicit hmc_hdf5(const char* filename = "hmcsim.h5");
  ~hmc_hdf5(void);

  void execute(unsigned linkTypeId, unsigned linkIntTypeId,
               uint64_t cycle, uint64_t phyPktAddr,
               int fromId, int toId,
               uint64_t header, uint64_t tail);
};

#endif /* #ifndef _HMC_TRACE_HDF5_H_ */
