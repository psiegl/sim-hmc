#include <cassert>
#include <cstdint>
#include "hmc_cube.h"
#include "hmc_link.h"
#include "hmc_link_queue.h"
#include "hmc_connection.h"
#include "hmc_packet.h"
#include "hmc_notify.h"
#include "hmc_macros.h"
#include "hmc_decode.h"
#include "hmc_quad.h"

hmc_conn_part::hmc_conn_part(unsigned id, hmc_notify *notify, hmc_cube *cub) :
  hmc_notify_cl(),
  hmc_module(),
  id(id),
  cub(cub),
  links_notify(id, notify, this),
  linkrxbuf_notify(id, notify, this),
  roundRobinSchedule(0x0),
  cyclesBlocked(0x0)
{
  for (unsigned i = 0; i < HMC_JTL_ALL_LINKS; i++)
    this->links[i] = nullptr;
}

hmc_conn_part::~hmc_conn_part(void)
{
}

bool hmc_conn_part::_set_link(unsigned notifyid, unsigned id, hmc_link *link)
{
  if (!this->links[notifyid]) {
    this->links[notifyid] = link;
    link->set_ilink_notify(notifyid, id, &this->links_notify, &this->linkrxbuf_notify);
    return true;
  }
  return false;
}

unsigned hmc_conn_part::decode_link_of_packet(char *packet)
{
  uint64_t header = HMC_PACKET_HEADER(packet);
  unsigned p_cubId, cubId = this->cub->get_id();
  if (HMCSIM_PACKET_IS_RESPONSE(header)) {
    unsigned slid = (unsigned)HMCSIM_PACKET_RESPONSE_GET_SLID(header);
    p_cubId = this->cub->slid_to_cubid(slid);
    if (p_cubId == cubId) {
      unsigned p_quadId = this->cub->slid_to_quadid(slid);
      if (p_quadId == this->id)
        return HMC_JTL_EXT_LINK(0);
      else
        return HMC_JTL_RING_LINK(this->routing(p_quadId));
    }
  }
  else {
    p_cubId = (unsigned)HMCSIM_PACKET_REQUEST_GET_CUB(header);
    if (p_cubId == cubId) {
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
  if (HMC_JTL_RING_LINK(0) <= ext_id && ext_id < HMC_JTL_RING_LINK(HMC_NUM_QUADS)) {
    // extract id -> set routing of ring -> insert id
    ext_id = HMC_JTL_RING_LINK(this->routing(ext_id - HMC_JTL_RING_LINK(0)));
  }
  return ext_id;
}

void hmc_conn_part::clock(void)
{
#ifndef HMC_USES_NOTIFY
  for (unsigned i = 0; i < HMC_JTL_ALL_LINKS; i++) {
    if (this->links[i] != nullptr)
      this->links[i]->clock();
  }

  unsigned i = this->roundRobinSchedule;
  if (this->links[i] != nullptr) {
    hmc_link_fifo *rx = this->links[i]->get_rx_fifo_out();
    unsigned packetleninbit;
    char *packet = rx->front(&packetleninbit);
    if (packet != nullptr) {
      unsigned linkId = this->decode_link_of_packet(packet);
      hmc_link *next_link = this->links[linkId];
      assert(next_link != nullptr);
      hmc_link_queue *tx = next_link->get_tx();
      assert(tx != nullptr);
      if (tx->push_back(packet, packetleninbit))
        rx->pop_front();
    }
  }
#else
  unsigned notifymap = this->links_notify.get_notification();
  for (unsigned i, lid = i = __builtin_ctzl(notifymap);
       notifymap >>= lid;
       lid = __builtin_ctzl(notifymap >>= 1),
       i += (lid + 1)) {
    this->links[i]->clock();
  }

//  if (cyclesBlocked) {
//    cyclesBlocked--;
//    return;
//  }

  notifymap = this->linkrxbuf_notify.get_notification();
  // round robin ...
  unsigned notifymap_p0 = notifymap >> this->roundRobinSchedule;
  unsigned notifymap_p1 = notifymap & ((0x1 << this->roundRobinSchedule) - 1);
  notifymap = (notifymap_p1 << (HMC_JTL_ALL_LINKS - this->roundRobinSchedule)) | notifymap_p0;

  unsigned i = __builtin_ctzl(notifymap);
  if (notifymap >>= i) {
    // round robin ...
    if ((i += this->roundRobinSchedule) >= HMC_JTL_ALL_LINKS)
      i -= HMC_JTL_ALL_LINKS;

    this->roundRobinSchedule = i;   // last one scheduled will be saved ..

    hmc_link_fifo *rx = this->links[i]->get_rx_fifo_out();
    unsigned packetleninbit;
    char *packet = rx->front(&packetleninbit);
    assert(packet != nullptr);

    unsigned linkId = this->decode_link_of_packet(packet);
    hmc_link *next_link = this->links[linkId];
    assert(next_link != nullptr);
    hmc_link_queue *tx = next_link->get_tx();
    assert(tx != nullptr);
    if (tx->push_back(packet, packetleninbit)) {
      rx->pop_front();
//      this->cyclesBlocked = packetleninbit / 384;
    }
  }
#endif /* #ifndef HMC_USES_NOTIFY */

  if (++this->roundRobinSchedule >= HMC_JTL_ALL_LINKS)
    this->roundRobinSchedule = 0x0;
}
bool hmc_conn_part::notify_up(unsigned id)
{
#ifdef HMC_USES_NOTIFY
  return (!this->links_notify.get_notification()
          && !this->linkrxbuf_notify.get_notification());
#else
  return true;
#endif /* #ifdef HMC_USES_NOTIFY */
}

void hmc_conn::clock(void)
{
#ifdef HMC_USES_NOTIFY
  unsigned notifymap = this->conn_notify.get_notification();
  for (unsigned i, lid = i = __builtin_ctzl(notifymap);
       notifymap >>= lid;
       lid = __builtin_ctzl(notifymap >>= 1),
       i += (lid + 1))
#else
  for (unsigned i = 0; i < HMC_NUM_QUADS; i++)
#endif /* #ifdef HMC_USES_NOTIFY */
  {
    this->conns[i]->clock();
  }
}

bool hmc_conn::notify_up(unsigned id)
{
#ifdef HMC_USES_NOTIFY
  return !this->conn_notify.get_notification();
#else
  return true;
#endif /* #ifdef HMC_USES_NOTIFY */
}
