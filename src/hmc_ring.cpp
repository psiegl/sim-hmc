#include <cassert>
#include <cstdint>
#include "hmc_cube.h"
#include "hmc_link.h"
#include "hmc_ring.h"
#include "hmc_queue.h"
#include "hmc_packet.h"
#include "hmc_notify.h"
#include "hmc_macros.h"
#include "hmc_decode.h"
#include "hmc_quad.h"

hmc_ring::hmc_ring(unsigned id, hmc_notify *notify, hmc_cube *cub) :
  hmc_notify_cl(),
  id(id),
  cub(cub),
  ringlinks_notify(id, notify, this),
  vaultlinks_notify(id, notify, this),
  extlinks_notify(id, notify, this),
  ext_link(nullptr)
{
}

hmc_ring::~hmc_ring(void)
{
}

int hmc_ring::set_ring_link(unsigned id, hmc_link *link)
{
  if (this->ring_link.find(id) == this->ring_link.end()) {
    this->ring_link[id] = link;
    link->set_ilink_notify(id, &ringlinks_notify);
    return 0;
  }
  return -1;
}

int hmc_ring::set_vault_link(unsigned id, hmc_link *link)
{
  if (this->vault_link.find(id) == this->vault_link.end()) {
    this->vault_link[id] = link;
    link->set_ilink_notify(id, &this->vaultlinks_notify);
    return 0;
  }
  return -1;
}

bool hmc_ring::set_ext_link(hmc_link *link)
{
  if (this->ext_link == nullptr) {
    this->ext_link = link;
    link->set_ilink_notify(0, &extlinks_notify);
    return true;
  }
  return false;
}

hmc_link*hmc_ring::decode_link_of_packet(void *packet)
{
  uint64_t header = HMC_PACKET_HEADER(packet);
  unsigned p_cubId;
  if (HMCSIM_PACKET_IS_RESPONSE(header)) {
    unsigned slid = (unsigned)HMCSIM_PACKET_RESPONSE_SET_SLID(header);
    p_cubId = this->cub->slid_to_cubid(slid);
    if (p_cubId == this->cub->get_id()) {
      unsigned p_quadId = this->cub->slid_to_quadid(slid);
      if (p_quadId == this->id) {
        return this->ext_link;
      }
      else{
        // since this is a ring, we can't cross from 0 to 3 or 1 to 2.
        // we will route first up then right
        /*
           [00]  <- ^= 0b1 -> [01]

           ^=0b10             ^=0b10

           [10]  <- ^= 0b1 -> [11]

           scheme routes first among x-axis, than y-axis
         */
        unsigned shift = ((p_quadId ^ this->id) & 0b01);
        return this->ring_link[this->id ^ (0b10 >> shift)];
      }
    }
  }
  else{
    p_cubId = (unsigned)HMCSIM_PACKET_REQUEST_GET_CUB(header);
    if (p_cubId == this->cub->get_id()) {
      uint64_t addr = HMCSIM_PACKET_REQUEST_GET_ADRS(header);
      unsigned p_quadId = (unsigned)this->cub->HMCSIM_UTIL_DECODE_QUAD(addr);
      if (p_quadId == this->id) {
        unsigned p_vaultId = (unsigned)this->cub->HMCSIM_UTIL_DECODE_VAULT(addr);
        return this->vault_link[p_vaultId];
      }
      else{
        unsigned shift = ((p_quadId ^ this->id) & 0b01);
        return this->ring_link[this->id ^ (0b10 >> shift)];
      }
    }
  }
  std::cerr << "ToDo: Not implemented yet" << std::endl;
  return this->cub->ext_routing(p_cubId, this->id);
}

#include <iostream>
void hmc_ring::clock(void)
{
  // ToDo: just one packet or multiple?
  uint32_t notifymap = this->ringlinks_notify.get_notification();
  unsigned lid = __builtin_ctzl(notifymap); // ToDo: round robin? of all?
  for (unsigned i = lid; i < HMC_NUM_QUADS; i++) {
    if ((0x1 << i) & notifymap) {
      hmc_queue *queue = this->ring_link[i]->get_ilink();
      unsigned packetleninbit;
      void *packet = queue->front(&packetleninbit);
      if (packet == nullptr)
        continue;

      hmc_link *next_link = decode_link_of_packet(packet);
      assert(next_link != nullptr);
      hmc_queue *next_queue = next_link->get_olink();
      assert(next_queue != nullptr);
      if (!next_queue->has_space(packetleninbit)) {
        //std::cout << "----->>>>> noooooooo space!" << std::endl;
        continue;
      }

      next_queue->push_back(packet, packetleninbit);
      queue->pop_front();
    }
  }

  notifymap = this->extlinks_notify.get_notification();
  do {
    if (notifymap) {
      hmc_queue *queue = this->ext_link->get_ilink();
      unsigned packetleninbit;
      void *packet = queue->front(&packetleninbit);
      if (packet == nullptr)
        continue;

      hmc_link *next_link = decode_link_of_packet(packet);
      assert(next_link != nullptr);
      hmc_queue *next_queue = next_link->get_olink();
      assert(next_queue != nullptr);
      if (!next_queue->has_space(packetleninbit)) {
        //std::cout << "----->>>>> noooooooo space!" << std::endl;
        continue;
      }

      next_queue->push_back(packet, packetleninbit);
      queue->pop_front();
    }
  } while (0);

  notifymap = this->vaultlinks_notify.get_notification();
  lid = __builtin_ctzl(notifymap); // round robin?
  for (unsigned i = lid; i < HMC_NUM_VAULTS / HMC_NUM_QUADS; i++) {
    if ((0x1 << i) & notifymap) {
      hmc_queue *queue = this->vault_link[i]->get_ilink();
      unsigned packetleninbit;
      void *packet = queue->front(&packetleninbit);
      if (packet == nullptr)
        continue;

      hmc_link *next_link = decode_link_of_packet(packet);
      assert(next_link != nullptr);
      hmc_queue *next_queue = next_link->get_olink();
      assert(next_queue != nullptr);
      if (!next_queue->has_space(packetleninbit)) {
        //std::cout << "----->>>>> noooooooo space!" << std::endl;
        continue;
      }

      next_queue->push_back(packet, packetleninbit);
      queue->pop_front();
    }
  }
}

bool hmc_ring::notify_up(void)
{
  return(!this->extlinks_notify.get_notification() &&
         !this->ringlinks_notify.get_notification() &&
         !this->vaultlinks_notify.get_notification());
}
