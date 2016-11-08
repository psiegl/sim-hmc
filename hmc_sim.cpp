#include <iostream>
#include "hmc_sim.h"
#include "hmc_link.h"
#include "hmc_quad.h"

hmc_sim::hmc_sim(unsigned num_hmcs, unsigned num_slids,
                 unsigned num_links, unsigned capacity) :
  clk(0),
  cubes_notify(0, nullptr, this)
{
  this->config.num_hmcs = num_hmcs;
  this->config.num_slids = num_slids;
  this->config.num_links = num_links;

  if ((num_hmcs > HMC_MAX_DEVS) || (!num_hmcs)) {
    std::cerr << "INSUFFICIENT NUMBER DEVICES: between 1 to " << HMC_MAX_DEVS << " (" << num_hmcs << ")" << std::endl;
//    return HMC_ERROR_PARAMS;
  }

  if ((num_links != HMC_MIN_LINKS) && (num_links != HMC_MAX_LINKS)) {
    std::cerr << "INSUFFICIENT NUMBER LINKS: between " << HMC_MIN_LINKS << " to " << HMC_MAX_LINKS << " (" << num_links << ")" << std::endl;
//    return HMC_ERROR_PARAMS;
  }

  if ((capacity != HMC_MIN_CAPACITY) && (capacity != HMC_MAX_CAPACITY)) {
    std::cerr << "INSUFFICIENT AMOUNT CAPACITY: between " << HMC_MIN_CAPACITY << " to " << HMC_MAX_CAPACITY << " (" << capacity << ")" << std::endl;
//    return HMC_ERROR_PARAMS;
  }

  this->link_graph = new struct hmc_graph_t*[num_hmcs];
  for(unsigned i=0; i<num_hmcs; i++)
    this->link_graph[i] = new struct hmc_graph_t[num_hmcs];

  for(unsigned i=0; i<num_hmcs;i++)
    this->cubes[i] = new hmc_cube(i, &this->cubes_notify);
}

hmc_sim::~hmc_sim(void)
{
  for(unsigned i=0; i<this->config.num_hmcs; i++)
    delete[] this->link_graph[i];
  delete[] this->link_graph;

  for(unsigned i=0; i<this->config.num_hmcs;i++)
    delete this->cubes[i];

  std::list<hmc_link*>::iterator it;
  for(it = this->link_garbage.begin(); it != this->link_garbage.end(); ++it)
    delete[] *it;
}

struct hmc_graph_t** hmc_sim::get_link_graph(void)
{
  return this->link_graph;
}

bool hmc_sim::notify_up(void)
{
  return true;
}


bool hmc_sim::hmc_link_config(unsigned src_hmcId, unsigned src_linkId,
                              unsigned dest_hmcId, unsigned dest_linkId)
{

}

bool hmc_sim::hmc_link_to_slid(unsigned slidId, unsigned hmcId, unsigned linkId, hmc_link** slid)
{
  hmc_quad* quad = this->cubes[hmcId]->get_quad(linkId);

  hmc_link *link = new hmc_link[2];
  link[0].connect_linkports(&link[1]);
  this->link_garbage.push_back(link);
  // ToDo: maybe readjust here already!

  // notify all!
  for(unsigned i=0; i<this->config.num_hmcs; i++)
    this->cubes[i]->set_slid(slidId, hmcId, linkId);

  *slid = &link[1];

  return quad->set_ext_link(&link[0]);
}

bool hmc_sim::clock(void)
{
  this->clk++;
  uint32_t notifymap = this->cubes_notify.get_notification();
  if(notifymap)
  {
    unsigned lid = __builtin_ctzl(notifymap);
    for(unsigned h = lid; h < this->config.num_hmcs; h++ )
    {
      if((0x1 << h) & notifymap) {
        this->cubes[h]->clock();
      }
    }
    return true;
  }
  return false; // ToDo: this->clk
}
