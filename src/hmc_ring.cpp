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
  links_notify(id, notify, this)
{
  for (unsigned i = 0; i < HMC_JTL_ALL_LINKS; i++) {
    this->links[i] = nullptr;
  }
}

hmc_ring::~hmc_ring(void)
{
}

bool hmc_ring::set_link(unsigned lid, hmc_link *link)
{
  if (this->links[lid] == nullptr) {
    this->links[lid] = link;
    link->set_ilink_notify(lid, &this->links_notify);
    return true;
  }
  return false;
}

unsigned hmc_ring::decode_link_of_packet(void *packet)
{
  uint64_t header = HMC_PACKET_HEADER(packet);
  unsigned p_cubId;
  if (HMCSIM_PACKET_IS_RESPONSE(header)) {
    unsigned slid = (unsigned)HMCSIM_PACKET_RESPONSE_SET_SLID(header);
    p_cubId = this->cub->slid_to_cubid(slid);
    if (p_cubId == this->cub->get_id()) {
      unsigned p_quadId = this->cub->slid_to_quadid(slid);
      if (p_quadId == this->id) {
        return HMC_JTL_EXT_LINK;
      }
      else {
        return HMC_JTL_RING_LINK(this->routing(p_quadId));
      }
    }
  }
  else {
    p_cubId = (unsigned)HMCSIM_PACKET_REQUEST_GET_CUB(header);
    if (p_cubId == this->cub->get_id()) {
      uint64_t addr = HMCSIM_PACKET_REQUEST_GET_ADRS(header);
      unsigned p_quadId = (unsigned)this->cub->HMCSIM_UTIL_DECODE_QUAD(addr);
      if (p_quadId == this->id) {
        unsigned p_vaultId = (unsigned)this->cub->HMCSIM_UTIL_DECODE_VAULT(addr);
        return HMC_JTL_VAULT_LINK(p_vaultId);
      }
      else {
        return HMC_JTL_RING_LINK(this->routing(p_quadId));
      }
    }
  }
  unsigned ext_id = this->cub->ext_routing(p_cubId, this->id);
  // because the ext routing, does not now the ring routing .. handle it here ...
  if (HMC_JTL_RING_LINK(0) <= ext_id && ext_id < HMC_JTL_RING_LINK(HMC_NUM_QUADS - 1)) {
    // extract id -> set routing of ring -> insert id
    ext_id = HMC_JTL_RING_LINK(this->routing(ext_id - HMC_JTL_RING_LINK(0)));
  }
  return ext_id;
}

void hmc_ring::clock(void)
{
  // ToDo: just one packet or multiple?
  uint32_t notifymap = this->links_notify.get_notification();
  unsigned lid = __builtin_ctzl(notifymap); // ToDo: round robin? of all?
  for (unsigned i = lid; i < HMC_JTL_ALL_LINKS; i++) {
    if ((0x1 << i) & notifymap) {
      hmc_queue *queue = this->links[i]->get_ilink();
      unsigned packetleninbit;
      void *packet = queue->front(&packetleninbit);
      if (packet == nullptr)
        continue;

      hmc_link *next_link = this->links[decode_link_of_packet(packet)];
      assert(next_link != nullptr);
      hmc_queue *next_queue = next_link->get_olink();
      assert(next_queue != nullptr);
      if (!next_queue->has_space(packetleninbit))
        continue;

      next_queue->push_back(packet, packetleninbit);
      queue->pop_front();
    }
  }
}

bool hmc_ring::notify_up(void)
{
  return !this->links_notify.get_notification();
}
