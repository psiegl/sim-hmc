#include <iostream>
#include <cstring>
#include <cstdlib>
#include <sys/time.h>
#include <inttypes.h>
#include <math.h>
#include "src/hmc_sim.h"

int main(int argc, char* argv[])
{
  uint64_t issue_sum = 60000;
  float percentage_rd = 1.00;

  unsigned use_slids = 1;

  unsigned size = 256;
  hmc_rqst_t hmc_wr;
  hmc_rqst_t hmc_rd;
  switch(size) {
  case 16:
    hmc_wr = WR16;
    hmc_rd = RD16;
    break;
  case 32:
    hmc_wr = WR32;
    hmc_rd = RD32;
    break;
  case 48:
    hmc_wr = WR48;
    hmc_rd = RD48;
    break;
  case 64:
    hmc_wr = WR64;
    hmc_rd = RD64;
    break;
  case 80:
    hmc_wr = WR80;
    hmc_rd = RD80;
    break;
  case 96:
    hmc_wr = WR96;
    hmc_rd = RD96;
    break;
  case 112:
    hmc_wr = WR112;
    hmc_rd = RD112;
    break;
  case 128:
    hmc_wr = WR128;
    hmc_rd = RD128;
    break;
  default:
  case 256:
    hmc_wr = WR256;
    hmc_rd = RD256;
    break;
  }


  unsigned cubes = 4;
  unsigned capacity = 4;
  hmc_sim sim(cubes, 4, 4, capacity, HMCSIM_FULL_LINK_WIDTH, HMCSIM_BR30);
  unsigned destcub = 0; // 1

#ifdef HMC_USES_GRAPHVIZ
  // if GRAPHVIZ is enabled!
  hmc_notify* slidnotify = sim.hmc_get_slid_notify();
  if(slidnotify == nullptr) {
    std::cerr << "initialisation failed!" << std::endl;
    exit(-1);
  }
#else

  hmc_notify* slidnotify;
  for(unsigned slid=0; slid<use_slids; slid++)
    slidnotify = sim.hmc_define_slid(slid, 0, slid, HMCSIM_FULL_LINK_WIDTH, HMCSIM_BR15);

  bool ret = sim.hmc_set_link_config(0, 1, 1, 0, HMCSIM_FULL_LINK_WIDTH, HMCSIM_BR30);
  ret &= sim.hmc_set_link_config(0, 3, 1, 2, HMCSIM_FULL_LINK_WIDTH, HMCSIM_BR30);
  if (!ret || slidnotify == nullptr) {
    std::cerr << "link setup was not successful" << std::endl;
    return -1;
  }
#endif /* #ifdef HMC_USES_GRAPHVIZ */

  srand(100);

  uint64_t issue_writes = issue_sum * (1-percentage_rd);
  uint64_t issue_reads = issue_sum - issue_writes;


  unsigned clks = 0;
  uint64_t send_ctr = 0;
  uint64_t recv_ctr = 0;
  unsigned *track = new unsigned[issue_sum];
  char packet[(17*FLIT_WIDTH) / (sizeof(char)*8)];
  char retpacket[17*FLIT_WIDTH / (sizeof(char)*8)];

  struct timeval t1, t2;
  gettimeofday(&t1, NULL);

  bool next_available = false;
  unsigned slidId = 0;
  do
  {
    if((issue_reads || issue_writes)
       && next_available == false)
    {
      unsigned addr = 0x0;
      // ToDo: address alignment depending on size!
      //addr |= (send_ctr & 0x1F) << 7; // quad + vault
      //addr |= (send_ctr & 0x3) << 10;

      // alignment to block size:
      addr &= ~((0x1ull << (unsigned)log2(size)) - 1);

      bool is_load = (bool)(rand() & 0x1);
      if(is_load && !issue_reads)
        is_load = false;
      if(!is_load && !issue_writes)
        is_load = true;

//      unsigned dram_hi = (send_ctr & 0b111) << 4;
//      unsigned dram_lo = (send_ctr >> 3) << (capacity == 8 ? 16 : 15);
//      addr |= dram_hi | dram_lo;
      if(is_load) {
        issue_reads--;
        sim.hmc_encode_pkt(destcub, addr, send_ctr /* tag */, hmc_rd, packet);
      }
      else {
        issue_writes--;
        sim.hmc_encode_pkt(destcub, addr, send_ctr /* tag */, hmc_wr, packet);
      }
      next_available = true;
    }

    if(next_available == true
       && sim.hmc_send_pkt(slidId, packet))
    {
      track[send_ctr] = clks;
      send_ctr++;
      next_available = false;
      if(++slidId >= use_slids)
        slidId = 0;
    }

    for(unsigned slid = 0; slid < use_slids; slid++)
      if(slidnotify->get_notification()
         && sim.hmc_recv_pkt(slid, retpacket))
      {
        track[recv_ctr] = clks - track[recv_ctr];
        recv_ctr++;
      }
    if(recv_ctr >= issue_sum) // we always wait for all returns
      break;

    clks++;
    //if(clks > 311)
    //  exit(0);
    sim.clock();
  } while(true);

  gettimeofday(&t2, NULL);

  unsigned long long avg = 0;
  for(unsigned i=0; i<issue_sum; i++) {
    avg += track[i];
  }
  avg /= issue_sum;

  delete[] track;

  issue_writes = issue_sum * (1-percentage_rd);
  issue_reads = issue_sum - issue_writes;
  unsigned rd_size = size;
  unsigned wr_size = size;
  // ToDo: account not only for reads but also writes! and depending on the type send!
  std::cout << "issued: " << issue_sum << " (rds: " << issue_reads << ", wrs: "<< issue_writes<< ")" << std::endl;
  float freq = 0.8f; // we clk at the same frequency! otherwise: 0.8f
  std::cout << "done in " << clks << " clks, avg.: " << avg << std::endl;
  float rdbw_overhead = (((float)(rd_size+16)*8*issue_reads)/(clks*freq)); // Gbit/s
  float wrbw_overhead = (((float)(wr_size+(16*2))*8*issue_writes)/(clks*freq)); // Gbit/s  -> has two times the overhead!
  float bw_overhead = rdbw_overhead + wrbw_overhead;

  std::cout << "bw: " << bw_overhead << "Gbit/s, " << (bw_overhead/8) << "GB/s"  << std::endl;
  std::cout << "bw per lane: " << (bw_overhead/16) << "Gbit/s" << std::endl;


  double elapsedTime = (t2.tv_sec - t1.tv_sec) * 1000.0 + (t2.tv_usec - t1.tv_usec) / 1000.0; // ms
  std::cout << std::endl;
  std::cout << "Simulation time: " << elapsedTime << " ms, cycles: " << clks << ", " << (clks/elapsedTime) << " kHz" << std::endl;

  return 0;
}
