#include "hmc_route.h"
#include "hmc_cube.h"
#include "hmc_sim.h"

hmc_route::hmc_route(hmc_sim *sim) :
  sim(sim)
{
  unsigned cubes = this->sim->get_config()->num_cubes;
  this->link_graph = new hmc_graph_t[cubes];
  memset(this->link_graph, 0, sizeof(hmc_graph_t) * cubes);

  this->tbl = new hmc_route_t*[cubes];
  for (unsigned i = 0; i < cubes; i++)
    this->tbl[i] = NULL;
}

hmc_route::~hmc_route(void)
{
  delete[] this->link_graph;
}

void hmc_route::set_slid(unsigned slid, unsigned cubId, unsigned quadId)
{
  this->slidToCube[slid] = std::make_pair(cubId, quadId);
}


void hmc_route::hmc_routing_tables_visualize(void)
{
  std::cout << "Routing table\n" << std::endl;
  std::cout << "Src.  Dest.  Gateway  Hops\n" << std::endl;
  unsigned cubes = this->sim->get_config()->num_cubes;
  for (unsigned d = 0; d < cubes; d++) {
    hmc_cube *cube = this->sim->get_cube(d);
    for (unsigned t = 0; t < cubes; t++) {
      struct hmc_route_t *cur = cube->get_routingtbl()[ t ];
      while (cur != NULL) {
        std::cout << "  " << d << "     " << t << "       " << cur->next_dev << "   " << cur->hops << std::endl;
        cur = cur->next;
      }
    }
  }
}

void hmc_route::hmc_insert_route(hmc_cube *cube, unsigned cube_endId, struct hmc_route_t *route)
{
  struct hmc_route_t **cur = &cube->get_routingtbl()[ cube_endId ];
  struct hmc_route_t **pre = cur;
  while ((*cur) != NULL) {
    if ((*cur)->next_dev == route->next_dev) {
      if (route->hops >= (*cur)->hops) {
        delete route;
      }
      else {
        route->next = (*cur)->next;
        delete *cur;
        (*cur) = route;
      }
      return;
    }
    pre = cur;
    cur = &(*cur)->next;
  }
  if ((*pre) == NULL || pre == cur) {
    route->next = (*cur);
    (*pre) = route;
  }
  else {
    route->next = (*cur);
    (*pre)->next = route;
  }
}


int hmc_route::hmc_graph_search(unsigned start_id, unsigned i, unsigned first_hop, unsigned end_id, unsigned hop)
{
  if (i == end_id) {
    struct hmc_route_t *route = new struct hmc_route_t;
    route->hops = hop - 1;
    route->next_dev = first_hop;
    route->links = &this->sim->get_cube(start_id)->get_partial_link_graph(first_hop)->links;             // fixME
    this->hmc_insert_route(this->sim->get_cube(start_id), end_id, route);
//		printf("  found -> %d via %d to %d hops: %d  first link: %d (links: %d)\n", start_id, first_hop, end_id, hop - 1,
//						__builtin_ctz( hmc->link_graph[ start_id ][ first_hop ].links ), hmc_utils_popcount( sizeof(unsigned), hmc->link_graph[ start_id ][ first_hop ].links ) );
    return 1;
  }
  if (start_id == i && hop > 0)
    return 0;

  for (unsigned j = 0; j < this->sim->get_config()->num_cubes; j++) {
    if (i != j && this->sim->get_cube(i)->get_partial_link_graph(j)->links &&
        !this->sim->get_cube(i)->get_partial_link_graph(j)->visited) {
      first_hop = (!hop) ? j : first_hop;
//			printf("checking  (h%d) %d -- %d --> %d  (cur %d)\n", hop, start_id, first_hop, j, i );
      this->sim->get_cube(i)->get_partial_link_graph(j)->visited = 1;
      this->sim->get_cube(j)->get_partial_link_graph(i)->visited = 1;
      /*unsigned ret =*/ this->hmc_graph_search(start_id, j, first_hop, end_id, hop + 1);
      this->sim->get_cube(i)->get_partial_link_graph(j)->visited = 0;
      this->sim->get_cube(j)->get_partial_link_graph(i)->visited = 0;
//			if( ret )  // commented -> search for all
//				return 1;
    }
  }
  return 0;
}

// ToDo: load via JTAG!
void hmc_route::hmc_routing_tables_update(void)
{
  unsigned cubes = this->sim->get_config()->num_cubes;
  for (unsigned i = 0; i < cubes; i++) {
    for (unsigned j = 0; j < cubes; j++) {
      if (i != j)
        this->hmc_graph_search(i, i, i, j, 0);                           // ToDo: check in which direction routing is possible! NON SRC MODE <-> PASSTHROUGH

      // remove routing entries, if directly attached
      struct hmc_route_t *cur = this->sim->get_cube(i)->get_routingtbl()[j];
      while (cur != NULL) {
        if (cur->next_dev == j)
          break;
        cur = cur->next;
      }
      if (cur != NULL) {
        struct hmc_route_t *tmp = this->sim->get_cube(i)->get_routingtbl()[j];
        while (tmp != NULL) {
          struct hmc_route_t *next = tmp->next;
          if (tmp != cur)
            delete tmp;
          tmp = next;
        }
        this->sim->get_cube(i)->get_routingtbl()[j] = cur;
        cur->next = NULL;
      }
    }
    // sort by hops?
  }
}

void hmc_route::hmc_routing_cleanup(unsigned cubeId)
{
  struct hmc_route_t **route = this->sim->get_cube(cubeId)->get_routingtbl();
  for (unsigned i = 0; i < this->sim->get_config()->num_cubes; i++) {
    struct hmc_route_t *cur = route[ i ];
    while (cur != NULL) {
      struct hmc_route_t *pre = cur;
      cur = cur->next;
      delete pre;
    }
  }
}
