#include <cassert>
#include <iostream>
#include "../../src/hmc_sim.h" // C++
#include "../include/hmc_sim.h" // C Wrapper

extern "C" {
hmc_notify *slid_notifier;

int hmcsim_init(	struct hmcsim_t *hmc,
				uint32_t num_devs, 
				uint32_t num_links, 
				uint32_t num_vaults, 
				uint32_t queue_depth,
				uint32_t num_banks, 
				uint32_t num_drams, 
				uint32_t capacity, 
				uint32_t xbar_depth )
{
  assert(num_links == 2 || num_links == 4);
  hmc->hmcsim = (void*)new hmc_sim(num_devs, num_links, num_links, capacity, HMCSIM_FULL_LINK_WIDTH, HMCSIM_BR30);
  return 0;
}

int hmcsim_free( struct hmcsim_t *hmc )
{
  delete (hmc_sim*)hmc->hmcsim;
  return 0;
}

int hmcsim_link_config( struct hmcsim_t *hmc,
					uint32_t src_dev,
					uint32_t dest_dev, 
					uint32_t src_link,
					uint32_t dest_link, 
					hmc_link_def_t type )
{
  hmc_sim* hmcsim = (hmc_sim*)hmc->hmcsim;
  if( type == HMC_LINK_HOST_DEV ) { // host to dev
    hmc_notify *slid_not = hmcsim->hmc_define_slid(dest_link, dest_dev, dest_link, HMCSIM_FULL_LINK_WIDTH, HMCSIM_BR30);
    if(slid_not == nullptr)
      return -1;
    slid_notifier = slid_not;
    return 0;
  }
  else { // dev to dev
    return ! hmcsim->hmc_set_link_config(src_dev, src_link, dest_dev, dest_link, HMCSIM_FULL_LINK_WIDTH, HMCSIM_BR30);
  }
}

int hmcsim_trace_handle( struct hmcsim_t *hmc, FILE *tfile )
{

  return 0;
}

int hmcsim_trace_header( struct hmcsim_t *hmc )
{

  return 0;
}

int hmcsim_trace_level( struct hmcsim_t *hmc, uint32_t level )
{

  return 0;
}

int hmcsim_build_memrequest( struct hmcsim_t *hmc,
                                    uint8_t  cub,
                                    uint64_t addr,
                                    uint16_t  tag,
                                    hmc_rqst_t type,
                                    uint8_t link,
                                    uint64_t *payload,
                                    uint64_t *rqst_head,
                                    uint64_t *rqst_tail )
{
  hmc_sim* hmcsim = (hmc_sim*)hmc->hmcsim;
  hmcsim->hmc_encode_pkt(cub, addr, tag, type, (char*)rqst_head);
  return 0;
}

int hmcsim_decode_memresponse( struct hmcsim_t *hmc,
                                      uint64_t *packet,
                                      uint64_t *response_head,
                                      uint64_t *response_tail,
                                      hmc_response_t *type,
                                      uint8_t *length,
                                      uint16_t *tag,
                                      uint8_t *rtn_tag,
                                      uint8_t *src_link,
                                      uint8_t *rrp,
                                      uint8_t *frp,
                                      uint8_t *seq,
                                      uint8_t *dinv,
                                      uint8_t *errstat,
                                      uint8_t *rtc,
                                      uint32_t *crc )
{
  hmc_sim* hmcsim = (hmc_sim*)hmc->hmcsim;
  unsigned flits;
  hmcsim->hmc_decode_pkt((char*)packet, response_head, response_tail,
                         type, &flits, tag, src_link, rrp, frp,
                         seq, dinv, errstat, rtc, crc);
  if(length != NULL)
    *length = flits;
  return 0;
}

int hmcsim_send( struct hmcsim_t *hmc, unsigned slidId, uint64_t *packet )
{
  hmc_sim* hmcsim = (hmc_sim*)hmc->hmcsim;
  if( hmcsim->hmc_send_pkt(slidId, (char*)packet) )
    return HMC_OK;
  else
    return HMC_STALL;
}

int hmcsim_recv( struct hmcsim_t *hmc, uint32_t dev, uint32_t link, uint64_t *packet )
{
  hmc_sim* hmcsim = (hmc_sim*)hmc->hmcsim;
  if((slid_notifier->get_notification() & (1 << link))
     && hmcsim->hmc_recv_pkt(link, (char*)packet))
    return HMC_OK;
  else
    return HMC_STALL;
}

int hmcsim_clock( struct hmcsim_t *hmc )
{
  hmc_sim* hmcsim = (hmc_sim*)hmc->hmcsim;
  hmcsim->clock();
  return 0;
}

uint64_t hmcsim_get_clock( struct hmcsim_t *hmc )
{
  hmc_sim* hmcsim = (hmc_sim*)hmc->hmcsim;
  return hmcsim->hmc_get_clock();
}

int hmcsim_jtag_reg_read( struct hmcsim_t *hmc, uint32_t dev, uint64_t reg, uint64_t *result )
{
  hmc_sim* hmcsim = (hmc_sim*)hmc->hmcsim;
  return hmcsim->hmc_get_jtag_interface(dev)->jtag_reg_read(reg, result);
}

int hmcsim_jtag_reg_write( struct hmcsim_t *hmc, uint32_t dev, uint64_t reg, uint64_t value )
{
  hmc_sim* hmcsim = (hmc_sim*)hmc->hmcsim;
  return hmcsim->hmc_get_jtag_interface(dev)->jtag_reg_write(reg, value);
}

/* ----------------------------------------------------- HMCSIM_UTIL_SET_MAX_BLOCKSIZE */
/*
 * HMCSIM_UTIL_SET_MAX_BLOCKSIZE
 * See Table38 in the HMC Spec : pg 58
 *
 */
int hmcsim_util_set_max_blocksize( struct hmcsim_t *hmc, uint32_t dev, uint32_t bsize )
{
  uint64_t res;
  if (hmcsim_jtag_reg_read(hmc, dev, HMC_REG_AC, &res))
    return -1;

  unsigned v;
  switch (bsize) {
  case 32:
    v = 0x0;
    break;
  case 64:
    v = 0x1;
    break;
  case 128:
    v = 0x2;
    break;
  case 256:               // HMC spec v2.1 doesn't provide details about registers anymore, while 1.0, 1.1 don't contain 256
    v = 0x3;
    break;
  default:
    printf("ERROR: No supported BSIZE given %d\n", bsize);
    return -1;
  }

  res &= ~0xF;       // mask out Adress Mapping Mode of Address Configuration Register
  res |= v;

  return hmcsim_jtag_reg_write(hmc, dev, HMC_REG_AC, res);
}

/* ----------------------------------------------------- HMCSIM_UTIL_SET_ALL_MAX_BLOCKSIZE */
/*
 * HMCSIM_UTIL_SET_ALL_MAX_BLOCKSIZE
 * See Table38 in the HMC Spec : pg 58
 *
 */
int hmcsim_util_set_all_max_blocksize( struct hmcsim_t *hmc, unsigned devs, uint32_t bsize )
{
  unsigned i;
  for (i = 0; i < devs; i++) {
    if (hmcsim_util_set_max_blocksize(hmc, i, bsize))
      return -1;
  }
  return 0;
}

int hmcsim_util_get_max_blocksize( struct hmcsim_t *hmc, uint32_t dev, uint32_t *bsize )
{
  uint64_t res;
  if (hmcsim_jtag_reg_read(hmc, dev, HMC_REG_AC, &res))
    return -1;

  res &= 0xF;       // mask out Adress Mapping Mode of Address Configuration Register

  switch (res) {
  case 0x0:
    *bsize = 32;
  case 0x1:
    *bsize = 64;
  case 0x2:
    *bsize = 128;
  case 0x3:               // HMC spec v2.1 doesn't provide details about registers anymore, while 1.0, 1.1 don't contain 256
    *bsize = 256;
  default:
    printf("ERROR: No supported res given %d\n", res);
    return -1;
  }
  return 0;
}

int hmcsim_load_cmc( struct hmcsim_t *hmc, char *cmc_lib )
{

  return 0;
}
}

