#include <iostream>
#include "hmc_notify.h"
#include "hmc_ring.h"
#include "hmc_link.h"
#include "hmc_cube.h"
#include "hmc_sim.h"

int main(int argc, char* argv[])
{
  unsigned linkdepth = 1;
  unsigned linkwidth = 64;

  hmc_sim sim(1,2,4,8);
  hmc_link* slid;
  sim.hmc_link_to_slid(0, 0, 0, &slid);
  slid->re_adjust_links( linkwidth, linkdepth );

  unsigned packetlen= 256;
  void *packet = (void*) malloc (packetlen);
  unsigned issue = 2;
  unsigned ctr = 0;

  unsigned clks = 0;
  while(issue > ctr)
  {
    if(slid->get_olink()->has_space(packetlen))
    {
      slid->get_olink()->push_back( packet, packetlen );
      ctr++;
    }
    // set clk anyway
    clks++;
    sim.clock();
    if(clks > 200)
      return 0;
  }
  while(sim.clock()) {
    clks++;
  }

  std::cout << "clks: " << clks << std::endl;

  return 0;
}
