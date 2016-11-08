#ifndef _HMC_SIM_H_
#define _HMC_SIM_H_

#include <cstdint>
#include <map>
#include <list>
#include "config.h"
#include "hmc_cube.h"
#include "hmc_notify.h"

class hmc_link;

typedef enum{
  RD_RS     = 0x38,	/*! HMC-SIM: HMC_RESPONSE_T: READ RESPONSE */
  WR_RS     = 0x39,	/*! HMC-SIM: HMC_RESPONSE_T: WRITE RESPONSE */
  MD_RD_RS  = 0x3A,	/*! HMC-SIM: HMC_RESPONSE_T: MODE READ RESPONSE */
  MD_WR_RS  = 0x3B,	/*! HMC-SIM: HMC_RESPONSE_T: MODE WRITE RESPONSE */
  RSP_ERROR = 0x3E,	/*! HMC-SIM: HMC_RESPONSE_T: ERROR RESPONSE */
  RSP_NONE  = 0x00,	/*! HMC-SIM: HMC_RESPONSE_T: NO RESPONSE COMMAND */ // not really defined, but let's take it
  RSP_CMC				/*! HMC-SIM: HMC_RESPONSE_T: CUSTOM CMC RESPONSE */
} hmc_response_t;

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
