#include <iostream>
#include <cstring>
#include "src/hmc_sim.h"


int main(int argc, char* argv[])
{
  unsigned cubes = 2;
  unsigned capacity = 4;
  hmc_sim sim(cubes, 2, 4, capacity, HMCSIM_FULL_LINK_WIDTH, HMCSIM_BR30);
  unsigned slidId = 0;
  unsigned destcub = 0; // 1
  unsigned addr = 0b110000000000; // quad 3
  hmc_notify* slidnotify = sim.hmc_define_slid(slidId, 0, 0, HMCSIM_FULL_LINK_WIDTH, HMCSIM_BR30);

  bool ret = sim.hmc_set_link_config(0, 1, 1, 0, HMCSIM_FULL_LINK_WIDTH, HMCSIM_BR30);
  ret &= sim.hmc_set_link_config(0, 3, 1, 2, HMCSIM_FULL_LINK_WIDTH, HMCSIM_BR30);
  if (!ret || slidnotify == nullptr) {
    std::cerr << "link setup was not successful" << std::endl;
  }

  unsigned sendpacketleninbit = 2*FLIT_WIDTH;
  char packet[(17*FLIT_WIDTH) / 8];

  unsigned issue = 1000; //6002;
  unsigned send_ctr = 0;
  unsigned skip = 0;
  unsigned recv_ctr = 0 + skip;

  unsigned clks = 0;
  unsigned *track = new unsigned[issue];

  char retpacket[17*FLIT_WIDTH / 8];
  bool next_available = false;
  do
  {
    if(issue > send_ctr && next_available == false)
    {
      memset(packet, 0, (sendpacketleninbit / FLIT_WIDTH << 1) * sizeof(uint64_t));

      if(send_ctr < (issue /*- 2*/)) {
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
  for(unsigned i=0; i<issue-skip; i++) {
    avg += track[i];
  }
  avg /= (issue-skip);

  delete[] track;

  std::cout << "issued: " << issue << " (skip: " << skip << ")" << std::endl;
  float freq = 0.8f; // we clk at the same frequency! otherwise: 0.8f
  std::cout << "done in " << clks << " clks, avg.: " << avg << std::endl;
  float bw = (((float)(256+16)*8*(issue-skip))/(clks*freq)); // Gbit/s
  std::cout << "bw: " << bw << "GBit/s, " << (bw/8) << "GB/s"  << std::endl;
  std::cout << "bw per lane: " << (((float)(256+16)*8*(issue-skip))/(clks*freq*16)) << "GBit/s" << std::endl;

  return 0;
}
