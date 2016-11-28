#include <iostream>
#include <cstring>
#include "src/hmc_sim.h"


int main(int argc, char* argv[])
{
  unsigned cubes = 2;
  unsigned capacity = 4;
  hmc_sim sim(cubes, 2, 4, capacity, HMCSIM_FULL_LINK_WIDTH, HMCSIM_FULL_LINK_WIDTH);
  unsigned slidId = 0;
  unsigned destcub = 1;
  unsigned addr = 0b110000000000; // quad 3
  hmc_notify* slidnotify = sim.hmc_define_slid(slidId, 0, 0, HMCSIM_FULL_LINK_WIDTH);

  bool ret = sim.hmc_set_link_config(0, 1, 1, 0, HMCSIM_FULL_LINK_WIDTH);
  ret &= sim.hmc_set_link_config(0, 3, 1, 2, HMCSIM_FULL_LINK_WIDTH);
  if (!ret || slidnotify == nullptr) {
    std::cerr << "link setup was not successful" << std::endl;
  }

  unsigned sendpacketleninbit = 2*FLIT_WIDTH;
  char packet[sendpacketleninbit / 8];

  unsigned issue = 6002;
  unsigned send_ctr = 0;
  unsigned recv_ctr = 0;

  unsigned clks = 0;
  unsigned *track = new unsigned[issue];

  char retpacket[16*FLIT_WIDTH / 8];
  bool next_available = false;
  do
  {
    if(issue > send_ctr && next_available == false)
    {
      memset(packet, 0, (sendpacketleninbit / FLIT_WIDTH << 1) * sizeof(uint64_t));

      if(send_ctr < (issue - 2)) {
        unsigned dram_hi = (send_ctr & 0b111) << 4;
        unsigned dram_lo = (send_ctr >> 3) << (capacity == 8 ? 16 : 15);
        sim.hmc_encode_pkt(destcub, addr+dram_hi+dram_lo, 0, RD256, packet);
      }
      else
        sim.hmc_encode_pkt(destcub, addr, 0, WR64, packet);
      next_available = true;

    }
    if(next_available == true && sim.hmc_send_pkt(slidId, packet)) {
      track[send_ctr] = clks;
      send_ctr++;
      next_available = false;
    }
    if(slidnotify->get_notification() && sim.hmc_recv_pkt(slidId, retpacket))
    {
      track[recv_ctr] = clks - track[recv_ctr];
      recv_ctr++;
      if(recv_ctr >= issue)
        break;
    }
    // set clk anyway
    clks++;
    //if(clks > 311)
    //  exit(0);
    sim.clock();
  } while(true);

  unsigned long long avg = 0;
  for(unsigned i=0; i<issue; i++)
    avg += track[i];
  avg /= issue;

  delete[] track;
  std::cout << "done in " << clks << " clks, avg.: " << avg << std::endl;

  return 0;
}
