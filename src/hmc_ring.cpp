#include <cassert>
#include <cstdint>
#include "hmc_cube.h"
#include "hmc_link.h"
#include "hmc_link_queue.h"
#include "hmc_ring.h"
#include "hmc_packet.h"
#include "hmc_notify.h"
#include "hmc_macros.h"
#include "hmc_decode.h"
#include "hmc_quad.h"

hmc_ring::hmc_ring(unsigned id, hmc_notify *notify, hmc_cube *cub) :
  hmc_notify_cl(),
  id(id),
  cub(cub),
  links_notify(id, notify, this),
  links(HMC_JTL_ALL_LINKS, nullptr)
{
}

hmc_ring::~hmc_ring(void)
{
}

bool hmc_ring::_set_link(unsigned notifyid, unsigned id, hmc_link *link)
{
  if (!this->links[notifyid]) {
    this->links[notifyid] = link;
    link->set_ilink_notify(notifyid, id, &this->links_notify);
    return true;
  }
  return false;
}

unsigned hmc_ring::decode_link_of_packet(char *packet)
{
  uint64_t header = HMC_PACKET_HEADER(packet);
  unsigned p_cubId;
  if (HMCSIM_PACKET_IS_RESPONSE(header)) {
    unsigned slid = (unsigned)HMCSIM_PACKET_RESPONSE_GET_SLID(header);
    p_cubId = this->cub->slid_to_cubid(slid);
    if (p_cubId == this->cub->get_id()) {
      unsigned p_quadId = this->cub->slid_to_quadid(slid);
      if (p_quadId == this->id)
        return HMC_JTL_EXT_LINK(0);
      else
        return HMC_JTL_RING_LINK(this->routing(p_quadId));
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
      else
        return HMC_JTL_RING_LINK(this->routing(p_quadId));
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
#ifdef HMC_USES_NOTIFY
  uint32_t notifymap = this->links_notify.get_notification();
  unsigned lid = __builtin_ctzl(notifymap); // ToDo: round robin? of all?
#else
  unsigned lid = 0;
#endif /* #ifdef HMC_USES_NOTIFY */
  for (unsigned i = lid; i < HMC_JTL_ALL_LINKS; i++) {
#ifdef HMC_USES_NOTIFY
    if ((0x1 << i) & notifymap)
#else
    if (this->links[i] != nullptr)
#endif /* #ifdef HMC_USES_NOTIFY */
    {
      this->links[i]->clock(); // ToDo!

      hmc_link_buf *rx = this->links[i]->get_rx();
      unsigned packetleninbit;
      char *packet = rx->front(&packetleninbit);
      if (packet == nullptr)
        continue;

      unsigned linkId = decode_link_of_packet(packet);
      hmc_link *next_link = this->links[linkId];
      assert(next_link != nullptr);
      hmc_link_queue *tx = next_link->get_tx();
      assert(tx != nullptr);
      if (tx->push_back(packet, packetleninbit))
        rx->pop_front();
    }
  }
}
bool hmc_ring::notify_up(void)
{
#ifdef HMC_USES_NOTIFY
  return !this->links_notify.get_notification();
#else
  return true;
#endif /* #ifdef HMC_USES_NOTIFY */
}
