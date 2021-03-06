#include <iostream>
#include "hmc_route.h"
#include "hmc_connection.h"
#include "hmc_cube.h"

hmc_route::hmc_route(std::map<unsigned, hmc_cube*> *cubes, unsigned numcubes) :
  cubes(cubes)
{
  this->link_graph = new hmc_graph_t[numcubes];
  memset(this->link_graph, 0, sizeof(hmc_graph_t) * numcubes);

  this->tbl = new hmc_route_t*[numcubes];
  for (unsigned i = 0; i < numcubes; i++)
    this->tbl[i] = nullptr;
}

hmc_route::~hmc_route(void)
{
  delete[] this->link_graph;
  this->hmc_routing_cleanup();
}

void hmc_route::set_slid(unsigned slid, unsigned cubId, unsigned quadId)
{
  std::cout << "HMC_ROUTE: slid : " << slid << " cub " << cubId << " quad " << quadId << std::endl;
  this->slidToCube[slid] = std::make_pair(cubId, quadId);
}

unsigned hmc_route::ext_routing(unsigned destCubId, unsigned curQuadId)
{
  hmc_route_t *ext_route = this->tbl[destCubId];
  assert(ext_route); // should really not happen!

  unsigned i, mask = 0x1;
  for (; ext_route != nullptr; ext_route = ext_route->next) {
    for (i = 0; i < 4; i++, mask <<= 1) { // ToDo: max 4 links
      if ((*ext_route->links) & mask) {
        if (i == curQuadId)  // Mapping: each quad has its own single link (as of HMC2.1)
          return HMC_JTL_EXT_LINK(0);
        else
          return HMC_JTL_RING_LINK(i);
      }
    }
  }
  assert(0); // should not happen!
  return ~0x0;
}


void hmc_route::hmc_routing_tables_visualize(void)
{
  std::cout << "HMC_ROUTE: Routing table" << std::endl;
  std::cout << "HMC_ROUTE: Src.  Dest.  Gateway  Hops" << std::endl;
  unsigned numcubes = this->cubes->size();
  for (unsigned d = 0; d < numcubes; d++) {
    hmc_cube *cube = (*this->cubes)[d];
    for (unsigned t = 0; t < numcubes; t++) {
      hmc_route_t *cur = cube->get_routingtbl()[ t ];
      while (cur != NULL) {
        std::cout << "HMC_ROUTE:   " << d << "     " << t << "       " << cur->next_dev << "   " << cur->hops << std::endl;
        cur = cur->next;
      }
    }
  }
}

bool hmc_route::hmc_insert_route(hmc_cube *cube, unsigned cube_endId, hmc_route_t *route)
{
  hmc_route_t **cur = &cube->get_routingtbl()[ cube_endId ];
  hmc_route_t **pre = cur;
  while ((*cur) != nullptr) {
    if ((*cur)->next_dev == route->next_dev) {
      if (route->hops >= (*cur)->hops) {
        return false;
      }
      else {
        route->next = (*cur)->next;
        delete *cur;
        (*cur) = route;
        return true;
      }
    }
    pre = cur;
    cur = &(*cur)->next;
  }
  if ((*pre) == nullptr || pre == cur) {
    route->next = (*cur);
    (*pre) = route;
  }
  else {
    route->next = (*cur);
    (*pre)->next = route;
  }
  return true;
}


int hmc_route::hmc_graph_search(unsigned start_id, unsigned i, unsigned first_hop, unsigned end_id, unsigned hop)
{
  if (i == end_id) {
    hmc_route_t *route = new hmc_route_t[1];
    route->hops = hop - 1;
    route->next_dev = first_hop;
    route->links = &(*this->cubes)[start_id]->get_partial_link_graph(first_hop)->links;             // fixME
    if (!this->hmc_insert_route((*this->cubes)[start_id], end_id, route))
      delete[] route;
//		printf("  found -> %d via %d to %d hops: %d  first link: %d (links: %d)\n", start_id, first_hop, end_id, hop - 1,
//						__builtin_ctz( hmc->link_graph[ start_id ][ first_hop ].links ), hmc_utils_popcount( sizeof(unsigned), hmc->link_graph[ start_id ][ first_hop ].links ) );
    return 1;
  }
  if (start_id == i && hop > 0)
    return 0;

  for (unsigned j = 0; j < this->cubes->size(); j++) {
    if (i != j && (*this->cubes)[i]->get_partial_link_graph(j)->links &&
        !(*this->cubes)[i]->get_partial_link_graph(j)->visited) {
      first_hop = (!hop) ? j : first_hop;
//			printf("checking  (h%d) %d -- %d --> %d  (cur %d)\n", hop, start_id, first_hop, j, i );
      (*this->cubes)[i]->get_partial_link_graph(j)->visited = 1;
      (*this->cubes)[j]->get_partial_link_graph(i)->visited = 1;
      /*unsigned ret =*/ this->hmc_graph_search(start_id, j, first_hop, end_id, hop + 1);
      (*this->cubes)[i]->get_partial_link_graph(j)->visited = 0;
      (*this->cubes)[j]->get_partial_link_graph(i)->visited = 0;
//			if( ret )  // commented -> search for all
//				return 1;
    }
  }
  return 0;
}

// ToDo: load via JTAG!
void hmc_route::hmc_routing_tables_update(void)
{
  unsigned numcubes = this->cubes->size();
  for (unsigned i = 0; i < numcubes; i++) {
    for (unsigned j = 0; j < numcubes; j++) {
      if (i != j)
        this->hmc_graph_search(i, i, i, j, 0);                           // ToDo: check in which direction routing is possible! NON SRC MODE <-> PASSTHROUGH

      // remove routing entries, if directly attached
      hmc_route_t *cur = (*this->cubes)[i]->get_routingtbl()[j];
      while (cur != NULL) {
        if (cur->next_dev == j)
          break;
        cur = cur->next;
      }
      if (cur != NULL) {
        hmc_route_t *tmp = (*this->cubes)[i]->get_routingtbl()[j];
        while (tmp != NULL) {
          hmc_route_t *next = tmp->next;
          if (tmp != cur)
            delete tmp;
          tmp = next;
        }
        (*this->cubes)[i]->get_routingtbl()[j] = cur;
        cur->next = NULL;
      }
    }
    // sort by hops?
  }
}

void hmc_route::hmc_routing_cleanup(void)
{
  for (unsigned i = 0; i < this->cubes->size(); i++) {
    hmc_route_t *cur = this->tbl[ i ];
    while (cur != NULL) {
      hmc_route_t *pre = cur;
      cur = cur->next;
      delete[] pre;
    }
  }
  delete[] this->tbl;
}
