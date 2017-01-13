#include <iostream>
#include <cassert>
#include "hmc_cube.h"
#include "hmc_sim.h"
#include "hmc_link.h"
#include "hmc_quad.h"
#include "hmc_link_queue.h"
#include "hmc_link_buf.h"
#include "hmc_vault.h"
#include "hmc_sqlite3.h"

extern hmc_sqlite3 **hmc_trace_init;

hmc_sim::hmc_sim(unsigned num_hmcs, unsigned num_slids,
                 unsigned num_links, unsigned capacity,
                 unsigned ringbus_bitwidth, float ringbus_bitrate) :
  hmc_notify_cl(),
  clk(0),
  cubes_notify(0, nullptr, this),
  slidnotify(),
  num_slids(num_slids)
{
  if ((num_hmcs > HMC_MAX_DEVS) || (!num_hmcs)) {
    std::cerr << "INSUFFICIENT NUMBER DEVICES: between 1 to " << HMC_MAX_DEVS << " (" << num_hmcs << ")" << std::endl;
    throw false;
  }

  switch (num_links) {
  case HMC_MIN_LINKS:
  case HMC_MAX_LINKS:
    break;
  default:
    std::cerr << "INSUFFICIENT NUMBER LINKS: between " << HMC_MIN_LINKS << " to " << HMC_MAX_LINKS << " (" << num_links << ")" << std::endl;
    throw false;
  }

  if (num_slids > HMC_MAX_LINKS) {
    std::cerr << "INSUFFICIENT NUMBER SLIDS: between " << HMC_MIN_LINKS << " to " << HMC_MAX_LINKS << " (" << num_links << ")" << std::endl;
    throw false;
  }

  switch (capacity) {
  case HMC_MIN_CAPACITY:
  case HMC_MAX_CAPACITY:
    break;
  default:
    std::cerr << "INSUFFICIENT AMOUNT CAPACITY: between " << HMC_MIN_CAPACITY << " to " << HMC_MAX_CAPACITY << " (" << capacity << ")" << std::endl;
    throw false;
  }

  if (!*hmc_trace_init)
    *hmc_trace_init = new hmc_sqlite3("hmcsim.db");

  for (unsigned i = 0; i < num_hmcs; i++) {
    this->cubes[i] = new hmc_cube(i, &this->cubes_notify, ringbus_bitwidth, ringbus_bitrate, capacity, &this->cubes, num_hmcs, &this->clk);
    this->jtags[i] = new hmc_jtag(this->cubes[i]);
  }
}

hmc_sim::~hmc_sim(void)
{
  if (*hmc_trace_init)
    delete *hmc_trace_init;

  unsigned i = 0;
  for (std::map<unsigned, hmc_cube*>::iterator it = this->cubes.begin(); it != this->cubes.end(); ++it) {
    delete (*it).second;
    delete this->jtags[i++];
  }

  for (std::list<hmc_link*>::iterator it = this->link_garbage.begin(); it != this->link_garbage.end(); ++it) {
    delete *it;
  }
}

bool hmc_sim::notify_up(void)
{
  return true; // don't care
}

bool hmc_sim::hmc_set_link_config(unsigned src_hmcId, unsigned src_linkId,
                                  unsigned dst_hmcId, unsigned dst_linkId,
                                  unsigned bitwidth, float bitrate)
{
  hmc_quad *src_quad = this->cubes[src_hmcId]->get_quad(src_linkId);
  hmc_quad *dst_quad = this->cubes[dst_hmcId]->get_quad(dst_linkId);

  hmc_link *linkend0 = new hmc_link(&this->clk, src_quad, HMC_LINK_EXTERN, 0);
  hmc_link *linkend1 = new hmc_link(&this->clk, dst_quad, HMC_LINK_EXTERN, 0);
  linkend0->connect_linkports(linkend1);
  linkend0->adjust_both_linkends(bitwidth, bitrate);

  // adjust routing as of multiple HMCs
  this->cubes[src_hmcId]->get_partial_link_graph(dst_hmcId)->links |= (0x1 << src_linkId);
  this->cubes[dst_hmcId]->get_partial_link_graph(src_hmcId)->links |= (0x1 << dst_linkId);
  this->cubes[src_hmcId]->hmc_routing_tables_update(); // just one needed ...
  this->cubes[src_hmcId]->hmc_routing_tables_visualize();

  this->link_garbage.push_back(linkend0);
  this->link_garbage.push_back(linkend1);
  return true;
}

hmc_notify* hmc_sim::hmc_define_slid(unsigned slidId, unsigned hmcId, unsigned linkId,
                                     unsigned bitwidth, float bitrate)
{
  hmc_quad *quad = this->cubes[hmcId]->get_quad(linkId);

  hmc_link *linkend0 = new hmc_link(&this->clk, quad, HMC_LINK_EXTERN, 0);
  hmc_link *linkend1 = new hmc_link(&this->clk);
  linkend0->connect_linkports(linkend1);
  linkend0->adjust_both_linkends(bitwidth, bitrate);
  linkend1->set_ilink_notify(slidId, slidId, &this->slidnotify); // important 1!! -> will be return for slid

  this->link_garbage.push_back(linkend0);
  this->link_garbage.push_back(linkend1);

  // notify all!
  for (unsigned i = 0; i < this->cubes.size(); i++)
    this->cubes[i]->set_slid(slidId, hmcId, linkId);

  this->slids[slidId] = linkend1;
  return &this->slidnotify;
}

bool hmc_sim::hmc_send_pkt(unsigned slidId, char *pkt)
{
  uint64_t header = HMC_PACKET_HEADER(pkt);

  assert(HMCSIM_PACKET_REQUEST_GET_CUB(header) < this->cubes.size());
  assert(this->slids.find(slidId) != this->slids.end());

  unsigned flits = HMCSIM_PACKET_REQUEST_GET_LNG(header);
  unsigned flitwidthInBit = flits * FLIT_WIDTH;
  hmc_link_queue *slid = this->slids[slidId]->get_tx();
  if (!slid->has_space(flitwidthInBit)) // check if we have space!
    return false;

  char *packet = new char[flitwidthInBit / 8];
  memcpy(packet, pkt, flitwidthInBit / 8);
  packet[0] |= HMCSIM_PACKET_SET_REQUEST();

  unsigned len64bit = flits << 1;
  ((uint64_t*)packet)[len64bit - 1] &= ~(uint64_t)HMCSIM_PACKET_REQUEST_SET_SLID(~0x0); // mask out whatever is set for slid
  ((uint64_t*)packet)[len64bit - 1] |= (uint64_t)HMCSIM_PACKET_REQUEST_SET_SLID(slidId); // set slidId

  return slid->push_back(packet, flits * FLIT_WIDTH);
}

bool hmc_sim::hmc_recv_pkt(unsigned slidId, char *pkt)
{
  assert(this->slids.find(slidId) != this->slids.end());
  assert(pkt != nullptr);

  unsigned recvpacketleninbit;
  hmc_link_buf *rx = this->slids[slidId]->get_rx();
  char *packet = rx->front(&recvpacketleninbit);
  if (packet == nullptr)
    return false;

  rx->pop_front();
  memcpy(pkt, packet, recvpacketleninbit / 64);
  delete[] packet;
  return true;
}

void hmc_sim::hmc_decode_pkt(char *packet, uint64_t *response_head, uint64_t *response_tail,
                             hmc_response_t *type, unsigned *rtn_flits, uint16_t *tag,
                             uint8_t *slid, uint8_t *rrp, uint8_t *frp, uint8_t *seq,
                             uint8_t *dinv, uint8_t *errstat, uint8_t *rtc, uint32_t *crc)
{
  uint64_t header = HMC_PACKET_HEADER(packet);

  uint8_t flits = (uint8_t)HMCSIM_PACKET_RESPONSE_GET_LNG(header);
  uint64_t tail = ((uint64_t*)packet)[ (flits << 1) - 1 ];

  if (response_head != NULL)
    *response_head = header;
  if (response_tail != nullptr)
    *response_tail = tail;

  if (type != nullptr)
    *type = (hmc_response_t)HMCSIM_PACKET_RESPONSE_GET_CMD(header);
  if (rtn_flits != nullptr)
    *rtn_flits = flits;
  if (tag != nullptr)
    *tag = (uint16_t)HMCSIM_PACKET_RESPONSE_GET_TAG(header);
  if (slid != nullptr)
    *slid = (uint8_t)HMCSIM_PACKET_RESPONSE_GET_SLID(header);

  if (rrp != nullptr)
    *rrp = (uint8_t)HMCSIM_PACKET_RESPONSE_GET_RRP(tail);
  if (frp != nullptr)
    *frp = (uint8_t)HMCSIM_PACKET_RESPONSE_GET_FRP(tail);
  if (seq != nullptr)
    *seq = (uint8_t)HMCSIM_PACKET_RESPONSE_GET_SEQ(tail);
  if (dinv != nullptr)
    *dinv = (uint8_t)HMCSIM_PACKET_RESPONSE_GET_DINV(tail);
  if (errstat != nullptr)
    *errstat = (uint8_t)HMCSIM_PACKET_RESPONSE_GET_ERRSTAT(tail);
  if (rtc != nullptr)
    *rtc = (uint8_t)HMCSIM_PACKET_RESPONSE_GET_RTC(tail);
  if (crc != nullptr)
    *crc = (uint32_t)HMCSIM_PACKET_RESPONSE_GET_CRC(tail);
}

void hmc_sim::hmc_encode_pkt(unsigned cub, uint64_t addr,
                             uint16_t tag, hmc_rqst_t cmd, char *packet)
{
  unsigned flits = 0;
  switch (cmd) { // ToDo: would be nice, if their would be something common -> see hmc_process_packet.h
  case RD16:
  case RD32:
  case RD48:
  case RD64:
  case RD80:
  case RD96:
  case RD112:
  case RD128:
  case RD256:
  case MD_RD:
  case FLOW_NULL:   // right?
  case PRET:
  case TRET:
  case IRTRY:
  case INC8:
  case P_INC8:
    flits = 1;
    break;
  case WR16:
  case MD_WR:
  case BWR:
  case TWOADD8:
  case ADD16:
  case TWOADDS8R:
  case ADDS16R:
  case XOR16:
  case OR16:
  case NOR16:
  case AND16:
  case NAND16:
  case CASGT8:
  case CASGT16:
  case CASLT8:
  case CASLT16:
  case CASEQ8:
  case CASZERO16:
  case EQ8:
  case EQ16:
  case BWR8R:
  case SWAP16:
  case P_WR16:
  case P_BWR:
  case P_2ADD8:
  case P_ADD16:
    flits = 2;
    break;
  case WR32:
  case P_WR32:
    flits = 3;
    break;
  case WR48:
  case P_WR48:
    flits = 4;
    break;
  case WR64:
  case P_WR64:
    flits = 5;
    break;
  case WR80:
  case P_WR80:
    flits = 6;
    break;
  case WR96:
  case P_WR96:
    flits = 7;
    break;
  case WR112:
  case P_WR112:
    flits = 8;
    break;
  case WR128:
  case P_WR128:
    flits = 9;
    break;
  case WR256:
  case P_WR256:
    flits = 17;
    break;
  default:
    // ToDo: CMC!
    throw false;
  }

  uint64_t *pkt = (uint64_t*)packet;
  pkt[0] = 0x0ull;
  pkt[0] |= (uint64_t)HMCSIM_PACKET_REQUEST_SET_CMD(cmd);
  pkt[0] |= (uint64_t)HMCSIM_PACKET_REQUEST_SET_LNG(flits);
  pkt[0] |= (uint64_t)HMCSIM_PACKET_REQUEST_SET_TAG(tag);
  pkt[0] |= (uint64_t)HMCSIM_PACKET_REQUEST_SET_ADRS(addr);
  pkt[0] |= (uint64_t)HMCSIM_PACKET_REQUEST_SET_CUB(cub);

  pkt[2 * flits - 1] = 0x00ull;
  pkt[2 * flits - 1] |= (uint64_t)HMCSIM_PACKET_REQUEST_SET_RRP(hmcsim_rqst_getrrp());
  pkt[2 * flits - 1] |= (uint64_t)HMCSIM_PACKET_REQUEST_SET_FRP(hmcsim_rqst_getfrp());
  pkt[2 * flits - 1] |= (uint64_t)HMCSIM_PACKET_REQUEST_SET_SEQ(hmcsim_rqst_getseq(cmd));
  pkt[2 * flits - 1] |= (uint64_t)HMCSIM_PACKET_REQUEST_SET_Pb(0x1);
  //pkt[2 * flits - 1] |= (uint64_t)HMCSIM_PACKET_REQUEST_SET_SLID(slid);
  pkt[2 * flits - 1] |= (uint64_t)HMCSIM_PACKET_REQUEST_SET_RTC(hmcsim_rqst_getrtc());
  pkt[2 * flits - 1] |= (uint64_t)HMCSIM_PACKET_REQUEST_SET_CRC(hmcsim_crc32((unsigned char*)pkt, 2 * flits));  // crc32 calc. needs to be last of packet init!
}

void hmc_sim::clock(void)
{
  this->clk++;
  uint32_t notifymap = this->cubes_notify.get_notification();
  unsigned lid = __builtin_ctzl(notifymap);
  for (unsigned h = lid; h < this->cubes.size(); h++)
    if ((0x1 << h) & notifymap)
      this->cubes[h]->clock();

  notifymap = this->slidnotify.get_notification();
  lid = __builtin_ctzl(notifymap);
  for (unsigned slidId = lid; slidId < this->num_slids; slidId++)
    if ((0x1 << slidId) & notifymap)
      this->slids[slidId]->clock(); // ToDo
}
