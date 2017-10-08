#include "../include/hmc_sim.h"

int main(void)
{
  struct hmcsim_t hmc;
  hmcsim_init(&hmc, 1, 4, 64, 5, 16, 100, 4, 3);

  return 0;
}
