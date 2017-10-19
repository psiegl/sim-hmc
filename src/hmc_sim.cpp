#include <iostream>
#include <cassert>
#include "hmc_cube.h"
#include "hmc_sim.h"
#include "hmc_link.h"
#include "hmc_quad.h"
#include "hmc_link_queue.h"
#include "hmc_link_fifo.h"
#include "hmc_vault.h"
#include "hmc_connection.h"
#include "hmc_slid.h"
#ifdef HMC_LOGGING
# include "hmc_trace.h"
#endif /* #ifdef HMC_LOGGING */
#ifdef HMC_USES_GRAPHVIZ
# include "hmc_graphviz.h"
#endif /* #ifdef HMC_USES_GRAPHVIZ */

hmc_sim::hmc_sim(unsigned num_hmcs, unsigned num_slids,
                 unsigned num_links, unsigned capacity,
                 unsigned quadbus_bitwidth, float quadbus_bitrate) :
  hmc_notify_cl(),
  clk(0),
  cubes_notify(0, nullptr, this),
  slidnotify(),
  slidbufnotify(),
  num_slids(num_slids),
  num_links(num_links)
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

  if (num_slids > HMC_MAX_SLIDS) {
    std::cerr << "INSUFFICIENT NUMBER SLIDS: between 0 to " << HMC_MAX_SLIDS << " (" << num_slids << ")" << std::endl;
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

  for (unsigned i = 0; i < num_hmcs; i++) {
    this->cubes[i] = new hmc_cube(i, &this->cubes_notify, quadbus_bitwidth, quadbus_bitrate, capacity, &this->cubes, num_hmcs, &this->clk);
    this->jtags[i] = new hmc_jtag(this->cubes[i]);
  }

#ifdef HMC_LOGGING
  hmc_trace::trace_setup();
#endif /* #ifdef HMC_LOGGING */

  // set up the graph after everything else was set up!
#ifdef HMC_USES_GRAPHVIZ
  hmc_graphviz graph(this);
#endif /* #ifdef HMC_USES_GRAPHVIZ */
}

hmc_sim::~hmc_sim(void)
{
#ifdef HMC_LOGGING
  hmc_trace::trace_cleanup();
#endif /* #ifdef HMC_LOGGING */

  unsigned i = 0;
  for (std::map<unsigned, hmc_cube*>::iterator it = this->cubes.begin(); it != this->cubes.end(); ++it) {
    delete (*it).second;
    delete this->jtags[i++];
  }

  for (std::list<hmc_link*>::iterator it = this->link_garbage.begin(); it != this->link_garbage.end(); ++it) {
    delete *it;
  }
  for (std::list<hmc_slid*>::iterator it = this->slidModule_garbage.begin(); it != this->slidModule_garbage.end(); ++it) {
    delete *it;
  }
}

bool hmc_sim::notify_up(unsigned id)
{
  return true; // don't care
}

bool hmc_sim::hmc_set_link_config(unsigned src_hmcId, unsigned src_linkId,
                                  unsigned dst_hmcId, unsigned dst_linkId,
                                  unsigned bitwidth, float bitrate)
{
  if (src_linkId >= this->num_links) {
    std::cerr << "(SRC) Defined link heigher than amount of links defined (" << src_linkId << " / " << this->num_links << ")" << std::endl;
    return false;
  }
  if (src_hmcId >= this->cubes.size()) {
    std::cerr << "(SRC) Defined hmc heigher than amount of hmcs defined (" << src_hmcId << " / " << this->cubes.size() << ")" << std::endl;
    return false;
  }
  if (dst_linkId >= this->num_links) {
    std::cerr << "(DST) Defined link heigher than amount of links defined (" << dst_linkId << " / " << this->num_links << ")" << std::endl;
    return false;
  }
  if (dst_hmcId >= this->cubes.size()) {
    std::cerr << "(DST) Defined hmc heigher than amount of hmcs defined (" << dst_hmcId << " / " << this->cubes.size() << ")" << std::endl;
    return false;
  }

  hmc_cube *src_cub = this->cubes[src_hmcId];
  hmc_conn_part *src_quad = src_cub->get_conn(src_linkId);
  hmc_cube *dst_cub = this->cubes[dst_hmcId];
  hmc_conn_part *dst_quad = dst_cub->get_conn(dst_linkId);

  hmc_link *linkend0 = new hmc_link(&this->clk, HMC_LINK_EXTERN, src_quad, src_cub, 0);
  hmc_link *linkend1 = new hmc_link(&this->clk, HMC_LINK_EXTERN, dst_quad, dst_cub, 0);
  linkend0->connect_linkports(linkend1);
  linkend0->adjust_both_linkends(bitwidth, bitrate, FLIT_WIDTH * RETRY_BUFFER_FLITS);

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
                                     unsigned lanes, float bitrate)
{
  if (slidId >= this->num_slids) {
    std::cerr << "Defined slid heigher than amount of slids defined (" << slidId << " / " << this->num_slids << ")" << std::endl;
    return nullptr;
  }
  if (linkId >= this->num_links) {
    std::cerr << "Defined link heigher than amount of links defined (" << linkId << " / " << this->num_links << ")" << std::endl;
    return nullptr;
  }
  if (hmcId >= this->cubes.size()) {
    std::cerr << "Defined hmc heigher than amount of hmcs defined (" << hmcId << " / " << this->cubes.size() << ")" << std::endl;
    return nullptr;
  }
  if (this->slids.find(slidId) != this->slids.end()) {
    std::cerr << "ERROR: slid already set!" << std::endl;
    return nullptr;
  }

  hmc_cube *cub = this->cubes[hmcId];
  hmc_conn_part *quad = cub->get_conn(linkId);

  hmc_slid *slid_module = new hmc_slid(slidId);
  hmc_link *linkend0 = new hmc_link(&this->clk, HMC_LINK_SLID, quad, cub, 0);
  hmc_link *linkend1 = new hmc_link(&this->clk, HMC_LINK_SLID, slid_module);
  linkend0->connect_linkports(linkend1);
  linkend0->adjust_both_linkends(lanes, bitrate, FLIT_WIDTH * RETRY_BUFFER_FLITS);
  linkend1->set_ilink_notify(slidId, slidId, &this->slidnotify, &this->slidbufnotify); // important 1!! -> will be return for slid

  this->link_garbage.push_back(linkend0);
  this->link_garbage.push_back(linkend1);
  this->slidModule_garbage.push_back(slid_module);

  // notify all!
  for (unsigned i = 0; i < this->cubes.size(); i++)
    this->cubes[i]->set_slid(slidId, hmcId, linkId);

  this->slids[slidId] = linkend1;
  return &this->slidbufnotify;
}

#ifdef HMC_USES_GRAPHVIZ
hmc_notify* hmc_sim::hmc_get_slid_notify(void)
{
  return &this->slidbufnotify;
}
#endif /* #ifdef HMC_USES_GRAPHVIZ */

bool hmc_sim::hmc_send_pkt(unsigned slidId, char *pkt)
{
  if (slidId >= this->num_slids) {
    std::cerr << "ERROR: defined slid heigher than amount of slids defined (" << slidId << " / " << this->num_slids << ")" << std::endl;
    return false;
  }
  if (this->slids.find(slidId) == this->slids.end()) {
    std::cerr << "ERROR: slid not set! Most likely initialisation error!" << std::endl;
    return false;
  }
  if (pkt == nullptr) {
    std::cerr << "ERROR: packet is nullptr!" << std::endl;
    return false;
  }

  uint64_t header = HMC_PACKET_HEADER(pkt);

  assert(HMCSIM_PACKET_REQUEST_GET_CUB(header) < this->cubes.size());

  unsigned flits = HMCSIM_PACKET_REQUEST_GET_LNG(header);
  unsigned flitwidthInBit = flits * FLIT_WIDTH;
  hmc_link_queue *slid = this->slids[slidId]->get_tx();
  if (!slid->has_space(flitwidthInBit)) // check if we have space!
    return false;

  char *packet = new char[flitwidthInBit / (sizeof(char) * 8)];
  memcpy(packet, pkt, flitwidthInBit / (sizeof(char) * 8));
  packet[0] |= HMCSIM_PACKET_SET_REQUEST(); // still a hack

  unsigned len64bit = flits << 1;
  ((uint64_t*)packet)[len64bit - 1] &= ~(uint64_t)HMCSIM_PACKET_REQUEST_SET_SLID(~0x0); // mask out whatever is set for slid
  ((uint64_t*)packet)[len64bit - 1] |= (uint64_t)HMCSIM_PACKET_REQUEST_SET_SLID(slidId); // set slidId

  return slid->push_back(packet, flits * FLIT_WIDTH);
}

bool hmc_sim::hmc_recv_pkt(unsigned slidId, char *pkt)
{
  if (slidId >= this->num_slids) {
    std::cerr << "ERROR: defined slid heigher than amount of slids defined (" << slidId << " / " << this->num_slids << ")" << std::endl;
    return false;
  }
  if (this->slids.find(slidId) == this->slids.end()) {
    std::cerr << "ERROR: slid not set! Most likely initialisation error!" << std::endl;
    return false;
  }

  unsigned recvpacketleninbit;
  hmc_link_fifo *rx = this->slids[slidId]->get_rx_fifo_out();
  char *packet = rx->front(&recvpacketleninbit);
  if (packet == nullptr)
    return false;

  rx->pop_front();
  if (pkt != nullptr)
    memcpy(pkt, packet, recvpacketleninbit / 8);
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

void hmc_sim::hmc_encode_pkt(unsigned cub, unsigned quad, unsigned vault, unsigned bank, unsigned dram,
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


  uint64_t addr = 0x0ull;
  addr |= ((hmc_cube*)this->cubes[cub])->HMCSIM_UTIL_ENCODE_QUAD(quad);
  addr |= ((hmc_cube*)this->cubes[cub])->HMCSIM_UTIL_ENCODE_VAULT(vault);
  addr |= ((hmc_cube*)this->cubes[cub])->HMCSIM_UTIL_ENCODE_BANK(bank);
  addr |= ((hmc_cube*)this->cubes[cub])->HMC_UTIL_ENCODE_DRAM_HI(dram);
  addr |= ((hmc_cube*)this->cubes[cub])->HMC_UTIL_ENCODE_DRAM_LO(dram);


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
  //pkt[2 * flits - 1] |= (uint64_t)HMCSIM_PACKET_REQUEST_SET_SLID(slid); // slid is set when send_pkt is issued
  pkt[2 * flits - 1] |= (uint64_t)HMCSIM_PACKET_REQUEST_SET_RTC(hmcsim_rqst_getrtc());
  pkt[2 * flits - 1] |= (uint64_t)HMCSIM_PACKET_REQUEST_SET_CRC(hmcsim_crc32((unsigned char*)pkt, 2 * flits));  // crc32 calc. needs to be last of packet init!
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
  //pkt[2 * flits - 1] |= (uint64_t)HMCSIM_PACKET_REQUEST_SET_SLID(slid); // slid is set when send_pkt is issued
  pkt[2 * flits - 1] |= (uint64_t)HMCSIM_PACKET_REQUEST_SET_RTC(hmcsim_rqst_getrtc());
  pkt[2 * flits - 1] |= (uint64_t)HMCSIM_PACKET_REQUEST_SET_CRC(hmcsim_crc32((unsigned char*)pkt, 2 * flits));  // crc32 calc. needs to be last of packet init!
}

void hmc_sim::clock(void)
{
  this->clk++;
#ifdef HMC_USES_NOTIFY
  unsigned notifymap = this->cubes_notify.get_notification();
  for (unsigned i, lid = i = __builtin_ctzl(notifymap);
       notifymap >>= lid;
       lid = __builtin_ctzl(notifymap >>= 1),
       i += (lid + 1))
#else
  for (unsigned i = 0; i < this->cubes.size(); i++)
#endif /* #ifdef HMC_USES_NOTIFY */
  {
    this->cubes[i]->clock();
  }

#ifdef HMC_USES_NOTIFY
  notifymap = this->slidnotify.get_notification();
  for (unsigned i, lid = i = __builtin_ctzl(notifymap);
       notifymap >>= lid;
       lid = __builtin_ctzl(notifymap >>= 1),
       i += (lid + 1))
#else
  for (unsigned i = 0; i < this->num_slids; i++)
#endif /* #ifdef HMC_USES_NOTIFY */
  {
#ifndef HMC_USES_NOTIFY
    if (this->slids[i] != nullptr)
#endif /* #ifndef HMC_USES_NOTIFY */
    {
      this->slids[i]->clock();
    }
  }
}
