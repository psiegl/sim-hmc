#ifndef _HMC_VAULT_H_
#define _HMC_VAULT_H_

#include "hmc_notify.h"

class hmc_link;
class hmc_cube;

typedef enum{
  WR16      = 0x08,	/* 16-BYTE WRITE REQUEST */
  WR32      = 0x09,	/* 32-BYTE WRITE REQUEST */
  WR48      = 0x0A,	/* 48-BYTE WRITE REQUEST */
  WR64      = 0x0B,	/* 64-BYTE WRITE REQUEST */
  WR80      = 0x0C,	/* 80-BYTE WRITE REQUEST */
  WR96      = 0x0D,	/* 96-BYTE WRITE REQUEST */
  WR112     = 0x0E,	/* 112-BYTE WRITE REQUEST */
  WR128     = 0x0F,	/* 128-BYTE WRITE REQUEST */
  MD_WR     = 0x10,	/* MODE WRITE REQUEST */
  BWR       = 0x11,	/* BIT WRITE REQUEST */
  TWOADD8   = 0x12,	/* DUAL 8-byte ADD IMMEDIATE */
  ADD16     = 0x13,	/* SINGLE 16-byte ADD IMMEDIATE */
  P_WR16    = 0x18,	/* 16-BYTE POSTED WRITE REQUEST */
  P_WR32    = 0x19,	/* 32-BYTE POSTED WRITE REQUEST */
  P_WR48    = 0x1A,	/* 48-BYTE POSTED WRITE REQUEST */
  P_WR64    = 0x1B,	/* 64-BYTE POSTED WRITE REQUEST */
  P_WR80    = 0x1C,	/* 80-BYTE POSTED WRITE REQUEST */
  P_WR96    = 0x1D,	/* 96-BYTE POSTED WRITE REQUEST */
  P_WR112   = 0x1E,	/* 112-BYTE POSTED WRITE REQUEST */
  P_WR128   = 0x1F,	/* 128-BYTE POSTED WRITE REQUEST */
  P_BWR     = 0x21,	/* POSTED BIT WRITE REQUEST */
  P_2ADD8   = 0x22,	/* POSTED DUAL 8-BYTE ADD IMMEDIATE */
  P_ADD16   = 0x23,	/* POSTED SINGLE 16-BYTE ADD IMMEDIATE */
  RD16      = 0x30,	/* 16-BYTE READ REQUEST */
  RD32      = 0x31,	/* 32-BYTE READ REQUEST */
  RD48      = 0x32,	/* 48-BYTE READ REQUEST */
  RD64      = 0x33,	/* 64-BYTE READ REQUEST */
  RD80      = 0x34,	/* 80-BYTE READ REQUEST */
  RD96      = 0x35,	/* 96-BYTE READ REQUEST */
  RD112     = 0x36,	/* 112-BYTE READ REQUEST */
  RD128     = 0x37,	/* 128-BYTE READ REQUEST */
  RD256     = 0x77,	/* 256-BYTE READ REQUEST */
  MD_RD     = 0x28,	/* MODE READ REQUEST */
  FLOW_NULL = 0x00,	/* NULL FLOW CONTROL */
  PRET      = 0x01,	/* RETRY POINTER RETURN FLOW CONTROL */
  TRET      = 0x02,	/* TOKEN RETURN FLOW CONTROL */
  IRTRY     = 0x03,	/* INIT RETRY FLOW CONTROL */

  /* -- version 2.0 Command Additions */
  WR256     = 0x4F,	/* 256-BYTE WRITE REQUEST */
  P_WR256   = 0x5F,	/* 256-BYTE POSTED WRITE REQUEST */
  TWOADDS8R = 0x52,	/* */
  ADDS16R   = 0x53,	/* */
  INC8      = 0x50,	/* 8-BYTE ATOMIC INCREMENT */
  P_INC8    = 0x54,	/* POSTED 8-BYTE ATOMIC INCREMENT */
  XOR16     = 0x40,	/* 16-BYTE ATOMIC XOR */
  OR16      = 0x41,	/* 16-BYTE ATOMIC OR */
  NOR16     = 0x42,	/* 16-BYTE ATOMIC NOR */
  AND16     = 0x43,	/* 16-BYTE ATOMIC AND */
  NAND16    = 0x44,	/* 16-BYTE ATOMIC NAND */
  CASGT8    = 0x60,	/* 8-BYTE COMPARE AND SWAP IF GT */
  CASGT16   = 0x62,	/* 16-BYTE COMPARE AND SWAP IF GT */
  CASLT8    = 0x61,	/* 8-BYTE COMPARE AND SWAP IF LT */
  CASLT16   = 0x63,	/* 16-BYTE COMPARE AND SWAP IF LT */
  CASEQ8    = 0x64,	/* 8-BYTE COMPARE AND SWAP IF EQ */
  CASZERO16 = 0x65,	/* 16-BYTE COMPARE AND SWAP IF ZERO */
  EQ8       = 0x69,	/* 8-BYTE ATOMIC EQUAL */
  EQ16      = 0x68,	/* 16-BYTE ATOMIC EQUAL */
  BWR8R     = 0x51,	/* 8-BYTE ATOMIC BIT WRITE WITH RETURN */
  SWAP16    = 0x6A,	/* 16-BYTE ATOMIC SWAP */
} hmc_rqst_t;

class hmc_vault {
private:
  hmc_link *link;
  hmc_cube *cube;

protected:
  bool hmcsim_process_rqst(void *packet);
  void hmcsim_packet_resp_len(hmc_rqst_t cmd, bool *no_response, unsigned *rsp_len);

public:
  hmc_vault(unsigned id, hmc_cube *cube, hmc_notify* notify, hmc_link *link);
  ~hmc_vault(void);
#ifndef HMC_USES_BOBSIM
  void clock(void);
#endif /* #ifndef HMC_USES_BOBSIM */
};

#endif /* #ifndef _HMC_VAULT_H_ */
