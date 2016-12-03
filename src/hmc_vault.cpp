#include <cassert>
#include <cstdint>
#include <cstring>
#include "hmc_vault.h"
#include "hmc_link.h"
#include "hmc_cube.h"
#include "hmc_macros.h"
#include "hmc_sim.h"



hmc_vault::hmc_vault(unsigned id, hmc_cube *cube, hmc_notify *notify, hmc_link *link) :
  link(link),
  cube(cube)
{
  this->link->set_ilink_notify(id, notify);

  for (unsigned i = 0; i < elemsof(this->jtl); i++)
    this->jtl[i] = nullptr; // is CMC

  for (unsigned i = 0; i < elemsof(this->jtli); i++)
    this->jtl[this->jtli[i].rsqt] = &this->jtli[i];
}

hmc_vault::~hmc_vault(void)
{
}

#ifndef HMC_USES_BOBSIM
//#include <iostream>
void hmc_vault::clock(void)
{
  unsigned packetleninbit;
  char *packet = this->link->get_ilink()->front(&packetleninbit);
  if (packet == nullptr)
    return;

  uint64_t header = HMC_PACKET_HEADER(packet);
  uint64_t addr = HMCSIM_PACKET_REQUEST_GET_ADRS(header);
  //unsigned bank = this->cube->HMCSIM_UTIL_DECODE_BANK(addr);

  //std::cout << "got packet!!! to bank " << bank << std::endl;
  // ToDo: Bank Conflict!

  if (this->hmcsim_process_rqst(packet)) {
    this->link->get_ilink()->pop_front();
    delete[] (char*)packet;
  }
}
#endif /* #ifndef HMC_USES_BOBSIM */


bool hmc_vault::hmcsim_packet_resp_len(hmc_rqst_t cmd, unsigned *rsp_len)
{
  if (jtl[cmd] != nullptr) {
    *rsp_len = jtl[cmd]->rsp_len;
    return !jtl[cmd]->rsp;
  }
  else {
#if 0
    switch (cmd) {
    /* begin CMC commands */
    default:
#if 0
      switch (dev->hmc->cmcs[ cmd ]->rsp_cmd) {
      case MD_RD_RS:
      case MD_WR_RS:
      case RSP_NONE:
        /* no response packet */
        response = false;
        break;
      default:
        break;
      }
#endif
      break;
    }
#endif
    return false;
  }
}

bool hmc_vault::hmcsim_process_rqst(void *packet)
{
  uint64_t rsp_payload[FLIT_WIDTH / 2 * HMC_MAX_FLITS_PER_PACKET];

  uint32_t error = 0x00;


  /*
   * Step 1: get the request
   *
   */
  uint64_t header = HMC_PACKET_HEADER(packet);
  uint64_t tail = HMC_PACKET_REQ_TAIL(packet);
//  unsigned length = HMCSIM_PACKET_REQUEST_GET_LNG(header);
  hmc_rqst_t cmd = (hmc_rqst_t)HMCSIM_PACKET_REQUEST_GET_CMD(header);


  /*
   * Step 2: decode it
   *
   */
  //uint64_t addr = (uint32_t)HMCSIM_PACKET_REQUEST_GET_ADRS(header);
  //uint32_t bank = (uint32_t)this->cube->HMCSIM_UTIL_DECODE_BANK(addr);


  unsigned rsp_flits;
  bool no_response = this->hmcsim_packet_resp_len(cmd, &rsp_flits);

  /*
   * Step 3: find a response slot
   *         if no slots available, then this operation must stall
   *
   */
  unsigned packetleninbit = rsp_flits * FLIT_WIDTH;
  hmc_queue *o_queue = this->link->get_olink();
  assert(o_queue);
  if (!no_response && !o_queue->has_space(packetleninbit)) {
//    HMCSIM_TRACE_STALL(dev->hmc, dev->id, 1);
    return false;
  }

  /*
   * Step 4: perform the op
   *
   */
  hmc_response_t rsp_cmd;
  if (jtl[cmd] != nullptr) {
    rsp_cmd = jtl[cmd]->rsp_cmd;

    // ToDo: if BOBSIM -> do it before issuing into bobsim! -> alignment!
    switch (cmd) {
    case WR48:
    case P_WR48:
    case RD48:
      if (this->cube->hmcsim_util_get_bsize() < 48) {
        error = 1;
        break;
      }
      break;

    case WR64:
    case P_WR64:
    case RD64:
      if (this->cube->hmcsim_util_get_bsize() < 64) {
        error = 1;
        break;
      }
      break;

    case WR80:
    case P_WR80:
    case RD80:
      if (this->cube->hmcsim_util_get_bsize() < 80) {
        error = 1;
        break;
      }
      break;

    case WR96:
    case P_WR96:
    case RD96:
      if (this->cube->hmcsim_util_get_bsize() < 96) {
        error = 1;
        break;
      }
      break;

    case WR112:
    case P_WR112:
    case RD112:
      if (this->cube->hmcsim_util_get_bsize() < 112) {
        error = 1;
        break;
      }
      break;

    case WR128:
    case P_WR128:
    case RD128:
      if (this->cube->hmcsim_util_get_bsize() < 128) {
        error = 1;
        break;
      }
      break;

    case WR256:
    case P_WR256:
    case RD256:
      if (this->cube->hmcsim_util_get_bsize() < 256) {
        error = 1;
        break;
      }
      break;
    default:
      break;
    }
  }
  else {
#if 0
    hmc_response_t tmp8 = 0x0;
    HMCSIM_PRINT_TRACE("HMCSIM_PROCESS_PACKET: PROCESSING CMC PACKET REQUEST\n");

    /* -- attempt to make a call to the cmc lib */
    error = hmcsim_process_cmc(dev->hmc,
                               cmd,
                               dev->id,
                               quad,
                               vault,
                               bank,
                               addr,
                               length,
                               packet[0],
                               tail,
                               &packet[1],
                               rsp_payload,
                               &rsp_flits,
                               &rsp_cmd,
                               &tmp8);
    /*
       MODE READ and WRITE request commands access the internal hardware mode and
       status registers within the logic layer of the HMC. Each mode request accesses up to 32
       contiguous bits of the register.
     */
    switch (rsp_cmd) {
    case MD_RD_RS:
    case MD_WR_RS:
    case RSP_NONE:
      break;
    default:
      rsp_cmd = tmp8;
      break;
    }
#endif
    rsp_cmd = RSP_ERROR;
  }

//  HMCSIM_TRACE_RQST(dev->hmc, dev->id, quad, vault, bank, addr, length, cmd_s);

  /*
   * Step 4: build and register the response with vault response queue
   *
   */
  if (!no_response) {
    /* -- build the response */
    uint64_t rsp_slid;
//#ifdef HMC_HAS_LOGIC
//    uint16_t logic_addr;
//    if (HMCSIM_PACKET_REQUEST_GET_FROM_LOGIC(tail)) {
//      logic_addr = HMCSIM_PACKET_REQUEST_GET_LOGIC_ADDR(tail);
//    }
//    else
//#endif /* #ifdef HMC_HAS_LOGIC */
    {
      rsp_slid = HMCSIM_PACKET_REQUEST_GET_SLID(tail);
    }
    uint64_t rsp_tag = HMCSIM_PACKET_REQUEST_GET_TAG(header);
    //uint64_t rsp_crc = HMCSIM_PACKET_REQUEST_GET_CRC( tail ); // ToDo: validate CRC!
    uint64_t rsp_rtc = HMCSIM_PACKET_REQUEST_GET_RTC(tail);
    uint64_t rsp_seq = HMCSIM_PACKET_REQUEST_GET_SEQ(tail);
    uint64_t rsp_frp = HMCSIM_PACKET_REQUEST_GET_FRP(tail);
    uint64_t rsp_rrp = HMCSIM_PACKET_REQUEST_GET_RRP(tail);

    char *response_packet = new char[rsp_flits * FLIT_WIDTH / 8];
    if (rsp_flits > 1)
      memcpy(&response_packet[1], rsp_payload, ((rsp_flits - 1) * FLIT_WIDTH) / 8);
    uint64_t *r_head = ((uint64_t*)response_packet);
    uint64_t *r_tail = &((uint64_t*)response_packet)[(rsp_flits << 1) - 1];

    /* -- packet head */
    *r_head = 0x0ull;
    *r_head |= HMCSIM_PACKET_RESPONSE_SET_CMD(rsp_cmd);
    *r_head |= HMCSIM_PACKET_RESPONSE_SET_LNG(rsp_flits);
    *r_head |= HMCSIM_PACKET_RESPONSE_SET_TAG(rsp_tag);
    *r_head |= HMCSIM_PACKET_RESPONSE_SET_AF(0);
//#ifdef HMC_HAS_LOGIC
//    if (HMCSIM_PACKET_REQUEST_GET_FROM_LOGIC(tail)) {
//      *r_head |= HMCSIM_PACKET_RESPONSE_SET_TO_LOGIC();
//      *r_head |= HMCSIM_PACKET_RESPONSE_SET_LOGIC_ADDR(logic_addr);  // [0:2 cub][0:1 logic]
//    }
//    else
//#endif /* #ifdef HMC_HAS_LOGIC */
    {
      *r_head |= HMCSIM_PACKET_RESPONSE_SET_SLID(rsp_slid);
      *r_head |= HMCSIM_PACKET_RESPONSE_SET_CUB(this->cube->get_id());   // FixMe: should it be the dest cube id?
    }

    *r_head |= HMCSIM_PACKET_SET_RESPONSE();   // not official
//#ifdef HMC_HAS_LOGIC
//#endif /* #ifdef HMC_HAS_LOGIC */

    /* -- packet tail */
    *r_tail = 0x0ull;
    *r_tail |= HMCSIM_PACKET_RESPONSE_SET_RRP(rsp_rrp);
    *r_tail |= HMCSIM_PACKET_RESPONSE_SET_FRP(rsp_frp);
    *r_tail |= HMCSIM_PACKET_RESPONSE_SET_SEQ(rsp_seq);
    *r_tail |= HMCSIM_PACKET_RESPONSE_SET_DINV(0);
    if (error)
      *r_tail |= HMCSIM_PACKET_RESPONSE_SET_ERRSTAT(0x1);    // ToDo: FixME with specific code
    *r_tail |= HMCSIM_PACKET_RESPONSE_SET_RTC(rsp_rtc);
    *r_tail |= HMCSIM_PACKET_RESPONSE_SET_CRC(hmcsim_crc32(response_packet, rsp_flits));    // ToDo: create CRC!

    /* -- register the response */
    o_queue->push_back(response_packet, packetleninbit);
  }  /* else, no response required, probably flow control */

  return true;
}
