#ifndef _HMC_ROUTE_H_
#define _HMC_ROUTE_H_

#include <map>
#include <cstring>
#include "hmc_link.h"
#include "hmc_macros.h"

class hmc_sim;
class hmc_cube;

struct hmc_graph_t {
    unsigned links;
    unsigned visited;
};

struct hmc_route_t {
  unsigned next_dev;
  unsigned hops;
  unsigned *links;
  hmc_route_t *next;
};

class hmc_route {
  hmc_sim *sim;

  std::map<unsigned, std::pair<unsigned,unsigned>> slidToCube;

  hmc_route_t ** tbl;
  struct hmc_graph_t* link_graph;

  void hmc_insert_route(hmc_cube *cube, unsigned cube_endId, struct hmc_route_t *route);
  int hmc_graph_search(unsigned start_id, unsigned i, unsigned first_hop, unsigned end_id, unsigned hop);

public:
  hmc_route(hmc_sim *sim);
  ~hmc_route(void);

  void hmc_routing_tables_visualize(void);
  void hmc_routing_tables_update(void);
  void hmc_routing_cleanup(unsigned cubeId);

  void set_slid(unsigned slid, unsigned cubId, unsigned quadId);

  ALWAYS_INLINE struct hmc_route_t** get_routingtbl(void)
  {
    return this->tbl;
  }
  ALWAYS_INLINE struct hmc_graph_t* get_partial_link_graph(unsigned id)
  {
    return &this->link_graph[id];
  }

  ALWAYS_INLINE unsigned slid_to_cubid(unsigned slid)
  {
    return this->slidToCube[slid].first;
  }

  ALWAYS_INLINE unsigned slid_to_quadid(unsigned slid)
  {
    return this->slidToCube[slid].second;
  }

  ALWAYS_INLINE hmc_link* ext_routing(unsigned cubId, unsigned curQuadId)
  {
    return nullptr;
  }
};

#endif /* #ifndef _HMC_ROUTE_H_ */
