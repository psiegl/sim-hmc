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
}

hmc_vault::~hmc_vault(void)
{
}

#ifndef HMC_USES_BOBSIM
#include <iostream>
void hmc_vault::clock(void)
{
  unsigned packetleninbit;
  void *packet = this->link->get_ilink()->front(&packetleninbit);
  if (packet == nullptr)
    return;

  uint64_t header = HMC_PACKET_HEADER(packet);
  uint64_t addr = HMCSIM_PACKET_REQUEST_GET_ADRS(header);
  unsigned bank = this->cube->HMCSIM_UTIL_DECODE_BANK(addr);

  std::cout << "got packet!!! to bank " << bank << std::endl;
  // ToDo: Bank Conflict!

  if (this->hmcsim_process_rqst(packet)) {
    this->link->get_ilink()->pop_front();
    delete (uint64_t*)packet;
  }
}
#endif /* #ifndef HMC_USES_BOBSIM */


void hmc_vault::hmcsim_packet_resp_len(hmc_rqst_t cmd, bool *no_response, unsigned *rsp_len)
{
  *no_response = false;
  *rsp_len = 0;
  switch (cmd) {
  case WR16:
  case WR32:
  case WR48:
  case WR64:
  case WR80:
  case WR96:
  case WR112:
  case WR128:
  case WR256:
  case MD_WR:
  case BWR:
    /* set the response length in flits */
    *rsp_len = 1;
    break;

  case TWOADD8:
  case ADD16:
    break;

  case P_WR16:
  case P_WR32:
  case P_WR48:
  case P_WR64:
  case P_WR80:
  case P_WR96:
  case P_WR112:
  case P_WR128:
  case P_WR256:
  case P_BWR:
  case P_2ADD8:
  case P_ADD16:
    /* set the response command */
    *no_response = true;
    break;

  case RD16:
    /* set the response length in FLITS */
    *rsp_len = 2;
    break;

  case RD32:
    /* set the response length in FLITS */
    *rsp_len = 3;
    break;

  case RD48:
    /* set the response length in FLITS */
    *rsp_len = 4;
    break;

  case RD64:
    /* set the response length in FLITS */
    *rsp_len = 5;
    break;

  case RD80:
    /* set the response length in FLITS */
    *rsp_len = 6;
    break;

  case RD96:
    /* set the response length in FLITS */
    *rsp_len = 7;
    break;

  case RD112:
    /* set the response length in FLITS */
    *rsp_len = 8;
    break;

  case RD128:
    /* set the response length in FLITS */
    *rsp_len = 9;
    break;

  case RD256:
    /* set the response length in FLITS */
    *rsp_len = 17;
    break;

  case MD_RD:
    /* set the response length in FLITS */
    *rsp_len = 2;
    break;

  case FLOW_NULL:
  case PRET:
  case TRET:
  case IRTRY:
    /* signal no response packet required */
    *no_response = true;
    break;

  case TWOADDS8R:
  case ADDS16R:
    /* set the response length in FLITS */
    *rsp_len = 2;
    break;

  case INC8:
    /* set the response length in FLITS */
    *rsp_len = 1;
    break;

  case P_INC8:
    /* set the response command */
    *no_response = true;
    break;

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
    /* set the response length in FLITS */
    *rsp_len = 2;
    break;

  case EQ8:
  case EQ16:
    /* set the response length in FLITS */
    *rsp_len = 1;
    break;

  case BWR8R:
  case SWAP16:
    /* set the response length in FLITS */
    *rsp_len = 2;
    break;

  /* begin CMC commands */
  default:
#if 0
    switch (dev->hmc->cmcs[ cmd ]->rsp_cmd) {
    case MD_RD_RS:
    case MD_WR_RS:
    case RSP_NONE:
      /* no response packet */
      *no_response = true;
      break;
    default:
      break;
    }
#endif
    break;
  }
}

bool hmc_vault::hmcsim_process_rqst(void *packet)
{
  uint64_t rsp_payload[HMC_MAX_UQ_PACKET - 2 * 2];

  uint32_t error = 0x00;
  hmc_response_t rsp_cmd = RSP_ERROR;


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


  bool no_response;
  unsigned rsp_flits;
  this->hmcsim_packet_resp_len(cmd, &no_response, &rsp_flits);

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
  const char *cmd_s = "";
  switch (cmd) {
  case WR16:
    cmd_s = "WR16";

    /* set the response command */
    rsp_cmd = WR_RS;
    break;

  case WR32:
    cmd_s = "WR32";

    /* set the response command */
    rsp_cmd = WR_RS;
    break;

  case WR48:
    /*
     * check to see if we exceed maximum block size
     *
     */
    if (this->cube->hmcsim_util_get_bsize() < 48) {
      error = 1;
      break;
    }

    cmd_s = "WR48";

    /* set the response command */
    rsp_cmd = WR_RS;
    break;

  case WR64:
    /*
     * check to see if we exceed maximum block size
     *
     */
    if (this->cube->hmcsim_util_get_bsize() < 64) {
      error = 1;
      break;
    }

    cmd_s = "WR64";

    /* set the response command */
    rsp_cmd = WR_RS;
    break;

  case WR80:
    /*
     * check to see if we exceed maximum block size
     *
     */
    if (this->cube->hmcsim_util_get_bsize() < 80) {
      error = 1;
      break;
    }

    cmd_s = "WR80";

    /* set the response command */
    rsp_cmd = WR_RS;
    break;

  case WR96:
    /*
     * check to see if we exceed maximum block size
     *
     */
    if (this->cube->hmcsim_util_get_bsize() < 96) {
      error = 1;
      break;
    }

    cmd_s = "WR96";

    /* set the response command */
    rsp_cmd = WR_RS;
    break;

  case WR112:
    /*
     * check to see if we exceed maximum block size
     *
     */
    if (this->cube->hmcsim_util_get_bsize() < 112) {
      error = 1;
      break;
    }

    cmd_s = "WR112";

    /* set the response command */
    rsp_cmd = WR_RS;
    break;

  case WR128:
    /*
     * check to see if we exceed maximum block size
     *
     */
    if (this->cube->hmcsim_util_get_bsize() < 128) {
      error = 1;
      break;
    }

    cmd_s = "WR128";

    /* set the response command */
    rsp_cmd = WR_RS;
    break;

  case WR256:
    /*
     * check to see if we exceed maximum block size
     *
     */
    if (this->cube->hmcsim_util_get_bsize() < 256) {
      error = 1;
      break;
    }

    cmd_s = "WR256";

    /* set the response command */
    rsp_cmd = WR_RS;
    break;

  case MD_WR:
    cmd_s = "MD_WR";

    /* set the response command */
    rsp_cmd = MD_WR_RS;
    break;

  case BWR:
    cmd_s = "BWR";

    /* set the response command */
    rsp_cmd = WR_RS;
    break;

  case TWOADD8:
    cmd_s = "2ADD8";

    /* set the response command */
    rsp_cmd = WR_RS;

    break;

  case ADD16:
    cmd_s = "ADD16";

    /* set the response command */
    rsp_cmd = WR_RS;
    break;

  case P_WR16:
    cmd_s = "P_WR16";
    break;

  case P_WR32:
    cmd_s = "P_WR32";
    break;

  case P_WR48:
    /*
     * check to see if we exceed maximum block size
     *
     */
    if (this->cube->hmcsim_util_get_bsize() < 48) {
      error = 1;
      break;
    }

    cmd_s = "P_WR48";
    break;

  case P_WR64:
    /*
     * check to see if we exceed maximum block size
     *
     */
    if (this->cube->hmcsim_util_get_bsize() < 64) {
      error = 1;
      break;
    }

    cmd_s = "P_WR64";

    break;

  case P_WR80:
    /*
     * check to see if we exceed maximum block size
     *
     */
    if (this->cube->hmcsim_util_get_bsize() < 80) {
      error = 1;
      break;
    }

    cmd_s = "P_WR80";
    break;

  case P_WR96:
    /*
     * check to see if we exceed maximum block size
     *
     */
    if (this->cube->hmcsim_util_get_bsize() < 96) {
      error = 1;
      break;
    }

    cmd_s = "P_WR96";
    break;

  case P_WR112:
    /*
     * check to see if we exceed maximum block size
     *
     */
    if (this->cube->hmcsim_util_get_bsize() < 112) {
      error = 1;
      break;
    }

    cmd_s = "P_WR112";
    break;

  case P_WR128:
    /*
     * check to see if we exceed maximum block size
     *
     */
    if (this->cube->hmcsim_util_get_bsize() < 128) {
      error = 1;
      break;
    }

    cmd_s = "P_WR128";
    break;

  case P_WR256:
    /*
     * check to see if we exceed maximum block size
     *
     */
    if (this->cube->hmcsim_util_get_bsize() < 256) {
      error = 1;
      break;
    }

    cmd_s = "P_WR256";
    break;

  case P_BWR:
    cmd_s = "P_BWR";
    break;

  case P_2ADD8:
    cmd_s = "P_2ADD8";
    break;

  case P_ADD16:
    cmd_s = "P_ADD16";
    break;

  case RD16:
    cmd_s = "RD16";

    /* set the response command */
    rsp_cmd = RD_RS;
    break;

  case RD32:
    cmd_s = "RD32";

    /* set the response command */
    rsp_cmd = RD_RS;
    break;

  case RD48:
    /*
     * check to see if we exceed maximum block size
     *
     */
    if (this->cube->hmcsim_util_get_bsize() < 48) {
      error = 1;
      break;
    }

    cmd_s = "RD48";

    /* set the response command */
    rsp_cmd = RD_RS;
    break;

  case RD64:
    /*
     * check to see if we exceed maximum block size
     *
     */
    if (this->cube->hmcsim_util_get_bsize() < 64) {
      error = 1;
      break;
    }

    cmd_s = "RD64";

    /* set the response command */
    rsp_cmd = RD_RS;
    break;

  case RD80:
    /*
     * check to see if we exceed maximum block size
     *
     */
    if (this->cube->hmcsim_util_get_bsize() < 80) {
      error = 1;
      break;
    }

    cmd_s = "RD80";

    /* set the response command */
    rsp_cmd = RD_RS;
    break;

  case RD96:
    /*
     * check to see if we exceed maximum block size
     *
     */
    if (this->cube->hmcsim_util_get_bsize() < 96) {
      error = 1;
      break;
    }

    cmd_s = "RD96";

    /* set the response command */
    rsp_cmd = RD_RS;
    break;

  case RD112:

    /*
     * check to see if we exceed maximum block size
     *
     */
    if (this->cube->hmcsim_util_get_bsize() < 112) {
      error = 1;
      break;
    }

    cmd_s = "RD112";

    /* set the response command */
    rsp_cmd = RD_RS;
    break;

  case RD128:
    /*
     * check to see if we exceed maximum block size
     *
     */
    if (this->cube->hmcsim_util_get_bsize() < 128) {
      error = 1;
      break;
    }

    cmd_s = "RD128";

    /* set the response command */
    rsp_cmd = RD_RS;
    break;

  case RD256:
    /*
     * check to see if we exceed maximum block size
     *
     */
    if (this->cube->hmcsim_util_get_bsize() < 256) {
      error = 1;
      break;
    }

    cmd_s = "RD256";

    /* set the response command */
    rsp_cmd = RD_RS;
    break;

  case MD_RD:
    cmd_s = "MD_RD";

    /* set the response command */
    rsp_cmd = MD_RD_RS;
    break;

  case FLOW_NULL:
    cmd_s = "FLOW_NULL";
    break;

  case PRET:
    cmd_s = "PRET";
    break;

  case TRET:
    cmd_s = "TRET";
    break;

  case IRTRY:
    cmd_s = "IRTRY";
    break;

  /* -- begin extended atomics -- */
  case TWOADDS8R:
    cmd_s = "2ADDS8R";

    /* set the response command */
    rsp_cmd = RD_RS;
    break;

  case ADDS16R:
    cmd_s = "ADDS16R";

    /* set the response command */
    rsp_cmd = RD_RS;
    break;

  case INC8:
    cmd_s = "INC8";

    /* set the response command */
    rsp_cmd = WR_RS;
    break;

  case P_INC8:
    cmd_s = "P_INC8";
    break;

  case XOR16:
    cmd_s = "XOR16";

    /* set the response command */
    rsp_cmd = RD_RS;
    break;

  case OR16:
    cmd_s = "OR16";

    /* set the response command */
    rsp_cmd = RD_RS;
    break;

  case NOR16:
    cmd_s = "NOR16";

    /* set the response command */
    rsp_cmd = RD_RS;
    break;

  case AND16:
    cmd_s = "AND16";

    /* set the response command */
    rsp_cmd = RD_RS;
    break;

  case NAND16:
    cmd_s = "NAND16";

    /* set the response command */
    rsp_cmd = RD_RS;
    break;

  case CASGT8:
    cmd_s = "CASGT8";

    /* set the response command */
    rsp_cmd = RD_RS;
    break;

  case CASGT16:
    cmd_s = "CASGT16";

    /* set the response command */
    rsp_cmd = RD_RS;
    break;

  case CASLT8:
    cmd_s = "CASLT8";

    /* set the response command */
    rsp_cmd = RD_RS;
    break;

  case CASLT16:
    cmd_s = "CASLT16";

    /* set the response command */
    rsp_cmd = RD_RS;
    break;

  case CASEQ8:
    cmd_s = "CASEQ8";

    /* set the response command */
    rsp_cmd = RD_RS;
    break;

  case CASZERO16:
    cmd_s = "CASZERO16";

    /* set the response command */
    rsp_cmd = RD_RS;
    break;

  case EQ8:
    cmd_s = "EQ8";

    /* set the response command */
    rsp_cmd = WR_RS;
    break;

  case EQ16:
    cmd_s = "EQ16";

    /* set the response command */
    rsp_cmd = WR_RS;
    break;

  case BWR8R:
    cmd_s = "BWR8R";

    /* set the response command */
    rsp_cmd = RD_RS;
    break;

  case SWAP16:
    cmd_s = "SWAP16";

    /* set the response command */
    rsp_cmd = RD_RS;
    break;

  default:
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
    break;
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

    uint64_t *response_packet = new uint64_t[rsp_flits << 1];
    if (rsp_flits > 1)
      memcpy(&response_packet[1], rsp_payload, ((rsp_flits - 1) * FLIT_WIDTH) / 8);
    uint64_t *r_head = response_packet;
    uint64_t *r_tail = &response_packet[(rsp_flits << 1) - 1];

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
      *r_head |= HMCSIM_PACKET_RESPONSE_SET_CUB(this->cube->get_id()); // FixMe: should it be the dest cube id?
    }

    *r_head |= HMCSIM_PACKET_SET_RESPONSE();  // not official
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
  }/* else, no response required, probably flow control */

  return true;
}
