#include <iostream>
#include <cstring>
#include "src/hmc_notify.h"
#include "src/hmc_ring.h"
#include "src/hmc_link.h"
#include "src/hmc_cube.h"
#include "src/hmc_sim.h"


int main(int argc, char* argv[])
{
  unsigned linkdepth = 1;
  unsigned linkwidth = 64;

  unsigned cubes = 2;
  hmc_sim sim(cubes,2,4,8);
  hmc_link* slid = sim.hmc_link_to_slid(0, 0, 0);
  slid->re_adjust_links( linkwidth, linkdepth );
  hmc_notify slidnotify;
  slid->set_ilink_notify(0, &slidnotify);

  sim.hmc_link_config(0, 1, 1, 0);
  sim.hmc_link_config(0, 3, 1, 2);

  unsigned packetlen= 256; // 1 flit = 128bit

  unsigned issue = 100;
  unsigned send_ctr = 0;
  unsigned recv_ctr = 0;

  unsigned clks = 0;
  unsigned *track = new unsigned[issue];
  do
  {
    if(issue > send_ctr && slid->get_olink()->has_space(packetlen))
    {
      uint64_t *packet = new uint64_t[packetlen/sizeof(uint64_t)];
      memset(packet, 0, packetlen);
      packet[0] = 0;
      packet[0] |= ((0x33) & 0x7F); // RD64
      packet[0] |= (((packetlen/128) & 0x1F) << 7);

      unsigned addr = 0b110000000000; // quad 3
      packet[0] |= (((uint64_t)((addr) & 0x3FFFFFFFFull)) << 24);

      //std::cout << "header in main: " << packet[0] << std::endl;

      slid->get_olink()->push_back( packet, packetlen );
      track[send_ctr] = clks;
      send_ctr++;
    }
    if(slidnotify.get_notification()) {
      unsigned packetleninbit;
      uint64_t* packet = (uint64_t*)slid->get_ilink()->front(&packetleninbit);
      if(packet != nullptr)
      {
        slid->get_ilink()->pop_front();
        //std::cout << "---->>> pop front! size: " << packetleninbit << std::endl;
        track[recv_ctr] = clks - track[recv_ctr];
        recv_ctr++;
        if(recv_ctr > issue)
          break;
      }
    }
    // set clk anyway
    clks++;
    //if(clks > 500)
    //  return 0;
  } while(sim.clock());

  std::cout << "clks: " << clks << std::endl;
  for(unsigned i=0; i<issue; i++) {
    std::cout << "pkt " << i << ". -> clks " << track[i] << std::endl;
  }

  delete[] track;
  std::cout << "done in " << clks << " clks " << std::endl;

  return 0;
}
