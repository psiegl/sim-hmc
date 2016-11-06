#include <cassert>
#include "hmc_ring.h"
#include "hmc_queue.h"
#include "hmc_packet.h"
#include "hmc_notify.h"

hmc_ring::hmc_ring(unsigned id, hmc_notify *notify) :
  id( id ),
  ring_notify( id, notify ),
  vault_notify( id, notify ),
  ext_notify( id, notify ),
  ext_link(nullptr)
{
}

hmc_ring::~hmc_ring(void)
{
}


int hmc_ring::set_ring_link(unsigned id, hmc_link* link)
{
  if(this->ring_link.find(id) == this->ring_link.end())
  {
    this->ring_link[id] = link;
    link->set_ilink_notify(id, &ring_notify);
    return 0;
  }
  return -1;
}

int hmc_ring::set_vault_link(unsigned id, hmc_link* link)
{
  if(this->vault_link.find(id) == this->vault_link.end())
  {
    this->vault_link[id] = link;
    link->set_ilink_notify(id, &this->vault_notify);
    return 0;
  }
  return -1;
}

int hmc_ring::set_ext_link(unsigned id, hmc_link* link)
{
  if(this->ext_link == nullptr)
  {
    this->ext_link = link;
    link->set_ilink_notify(id, &ext_notify);
    return 0;
  }
  return -1;
}

void hmc_ring::clock(void)
{
  printf("called!!\n");
  // ToDo: just one packet or multiple?
  uint32_t notifymap = this->ring_notify.get_notification();
  unsigned lid = __builtin_ctzl(notifymap); // round robin?
  for(unsigned i = lid; i < HMC_NUM_QUADS; i++ )
  {
    if((0x1 << i) & notifymap) {
      hmc_queue* queue = this->ring_link[i]->get_ilink();
      void *packet = queue->front();
      if(packet != nullptr) {
        printf("packet!\n");

        queue->pop_front();
      }
      else
        printf("nll\n");
      // decode .. and route further! if space available!

    }
  }
  notifymap = this->ext_notify.get_notification();
  if(notifymap)
  {
    hmc_queue* queue = this->ext_link->get_ilink();
    void *packet = queue->front();
    assert( packet );
    // decode .. and route further! if space available!
  }
  notifymap = this->vault_notify.get_notification();
  lid = __builtin_ctzl(notifymap); // round robin?
  for(unsigned i = lid; i < HMC_NUM_VAULTS / HMC_NUM_QUADS; i++ )
  {
    if((0x1 << i) & notifymap) {
      hmc_queue* queue = this->vault_link[i]->get_ilink();
      void *packet = queue->front();
      assert( packet );
      // decode .. and route further! if space available!
    }
  }
}
