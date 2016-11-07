#ifndef _HMC_SIM_H_
#define _HMC_SIM_H_

#include <cstdint>
#include <map>
#include <list>
#include "config.h"
#include "hmc_cube.h"
#include "hmc_notify.h"

class hmc_link;

struct hmc_graph_t {
    unsigned links;
    unsigned visited;
};

class hmc_sim : private hmc_notify_cl {

  uint64_t clk;
  hmc_notify cubes_notify;
  std::map<unsigned, hmc_cube*> cubes;

  struct hmc_graph_t **link_graph;

  struct {
    unsigned num_hmcs;
    unsigned num_slids;
    unsigned num_links;
  } config;

  std::list<hmc_link*> link_garbage;

public:
  hmc_sim(unsigned num_hmcs, unsigned num_slids,
          unsigned num_links, unsigned capacity);
  ~hmc_sim(void);

  bool hmc_link_config(unsigned src_hmcId, unsigned src_linkId,
                       unsigned dest_hmcId, unsigned dest_linkId);
  bool hmc_link_to_slid(unsigned slidId, unsigned hmcId, unsigned linkId, hmc_link** slid);

  bool clock(void);

  // intern needed
  struct hmc_graph_t** get_link_graph(void);

  bool notify_up(void);
};

#endif /* #ifndef _HMC_SIM_H_ */
