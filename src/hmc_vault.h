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

  /* -- CMC Types */
  CMC04     =    4,	/* CMC CMD=4 */
  CMC05     =    5,	/* CMC CMD=5 */
  CMC06     =    6,	/* CMC CMD=6 */
  CMC07     =    7,	/* CMC CMD=7 */
  CMC20     =   20,	/* CMC CMD=20 */
  CMC21     =   21,	/* CMC CMD=21 */
  CMC22     =   22,	/* CMC CMD=22 */
  CMC23     =   23,	/* CMC CMD=23 */
  CMC32     =   32,	/* CMC CMD=32 */
  CMC36     =   36,	/* CMC CMD=36 */
  CMC37     =   37,	/* CMC CMD=37 */
  CMC38     =   38,	/* CMC CMD=38 */
  CMC39     =   39,	/* CMC CMD=39 */
  CMC41     =   41,	/* CMC CMD=41 */
  CMC42     =   42,	/* CMC CMD=42 */
  CMC43     =   43,	/* CMC CMD=43 */
  CMC44     =   44,	/* CMC CMD=44 */
  CMC45     =   45,	/* CMC CMD=45 */
  CMC46     =   46,	/* CMC CMD=46 */
  CMC47     =   47,	/* CMC CMD=47 */
  CMC56     =   56,	/* CMC CMD=56 */
  CMC57     =   57,	/* CMC CMD=57 */
  CMC58     =   58,	/* CMC CMD=58 */
  CMC59     =   59,	/* CMC CMD=59 */
  CMC60     =   60,	/* CMC CMD=60 */
  CMC61     =   61,	/* CMC CMD=61 */
  CMC62     =   62,	/* CMC CMD=62 */
  CMC63     =   63,	/* CMC CMD=63 */
  CMC69     =   69,	/* CMC CMD=69 */
  CMC70     =   70,	/* CMC CMD=70 */
  CMC71     =   71,	/* CMC CMD=71 */
  CMC72     =   72,	/* CMC CMD=72 */
  CMC73     =   73,	/* CMC CMD=73 */
  CMC74     =   74,	/* CMC CMD=74 */
  CMC75     =   75,	/* CMC CMD=75 */
  CMC76     =   76,	/* CMC CMD=76 */
  CMC77     =   77,	/* CMC CMD=77 */
  CMC78     =   78,	/* CMC CMD=78 */
  CMC85     =   85,	/* CMC CMD=85 */
  CMC86     =   86,	/* CMC CMD=86 */
  CMC87     =   87,	/* CMC CMD=87 */
  CMC88     =   88,	/* CMC CMD=88 */
  CMC89     =   89,	/* CMC CMD=89 */
  CMC90     =   90,	/* CMC CMD=90 */
  CMC91     =   91,	/* CMC CMD=91 */
  CMC92     =   92,	/* CMC CMD=92 */
  CMC93     =   93,	/* CMC CMD=93 */
  CMC94     =   94,	/* CMC CMD=94 */
  CMC102    =  102,	/* CMC CMD=102 */
  CMC103    =  103,	/* CMC CMD=103 */
  CMC107    =  107,	/* CMC CMD=107 */
  CMC108    =  108,	/* CMC CMD=108 */
  CMC109    =  109,	/* CMC CMD=109 */
  CMC110    =  110,	/* CMC CMD=110 */
  CMC111    =  111,	/* CMC CMD=111 */
  CMC112    =  112,	/* CMC CMD=112 */
  CMC113    =  113,	/* CMC CMD=113 */
  CMC114    =  114,	/* CMC CMD=114 */
  CMC115    =  115,	/* CMC CMD=115 */
  CMC116    =  116,	/* CMC CMD=116 */
  CMC117    =  117,	/* CMC CMD=117 */
  CMC118    =  118,	/* CMC CMD=118 */
  CMC120    =  120,	/* CMC CMD=120 */
  CMC121    =  121,	/* CMC CMD=121 */
  CMC122    =  122,	/* CMC CMD=122 */
  CMC123    =  123,	/* CMC CMD=123 */
  CMC124    =  124,	/* CMC CMD=124 */
  CMC125    =  125,	/* CMC CMD=125 */
  CMC126    =  126,	/* CMC CMD=126 */
  CMC127    =  127	/* CMC CMD=127 */
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
