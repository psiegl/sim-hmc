#include "hmc_jtag.h"
#include "hmc_cube.h"

hmc_jtag::hmc_jtag(hmc_cube *cube) :
  cube(cube)
{
}

hmc_jtag::~hmc_jtag(void)
{
}

int hmc_jtag::jtag_reg_read(unsigned reg, uint64_t *result)
{
  return this->cube->hmcsim_reg_value_get_full(reg, result);
}

int hmc_jtag::jtag_reg_write(unsigned reg, uint64_t value)
{
  return this->cube->hmcsim_reg_value_set_full(reg, value);
}
