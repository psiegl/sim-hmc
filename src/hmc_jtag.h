#ifndef _HMC_JTAG_H_
#define _HMC_JTAG_H_

#include <stdint.h>

class hmc_cube;

class hmc_jtag {
private:
  hmc_cube *cube;

public:
  explicit hmc_jtag(hmc_cube *cube);
  ~hmc_jtag(void);

  int jtag_reg_read(unsigned reg, uint64_t* result);
  int jtag_reg_write(unsigned reg, uint64_t value);
};

#endif /* #ifndef _HMC_JTAG_H_ */
