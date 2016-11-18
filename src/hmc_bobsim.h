#ifndef _HMC_BOBSIM_H_
#define _HMC_BOBSIM_H_

#include <list>
#include "hmc_notify.h"
#include "hmc_macros.h"
#include "hmc_vault.h"
#include "../extern/bobsim/include/bob_c_wrapper.h"
#include "../extern/bobsim/include/bob_transaction.h"

class hmc_link;
class hmc_cube;

class hmc_bobsim : private hmc_notify_cl, private hmc_vault {
private:
  unsigned id;
  hmc_cube *cube;

  hmc_notify linknotify;
  hmc_link *link;

  unsigned bobnotify_ctr;
  hmc_notify bobnotify;
  BobWrapper *bobsim;

  std::list<void*>feedback_cache;

  enum TransactionType hmc_determineTransactionType(hmc_rqst_t cmd)
  {
    switch (cmd) {
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
      return DATA_READ;
    case FLOW_NULL:
    case PRET:
    case TRET:
    case IRTRY:
    case INC8:
    case P_INC8:
      return LOGIC_OPERATION;
    case WR16:
    case MD_WR:
    case BWR:
      return DATA_WRITE;
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
      return LOGIC_OPERATION;
    case BWR8R:
      return DATA_WRITE;
    case SWAP16:
      return LOGIC_OPERATION;
    case P_WR16:
    case P_BWR:
      return DATA_WRITE;
    case P_2ADD8:
    case P_ADD16:
      return LOGIC_OPERATION;
    case WR32:
    case P_WR32:
    case WR48:
    case P_WR48:
    case WR64:
    case P_WR64:
    case WR80:
    case P_WR80:
    case WR96:
    case P_WR96:
    case WR112:
    case P_WR112:
    case WR128:
    case P_WR128:
    case WR256:
    case P_WR256:
      return DATA_WRITE;
      break;
    default:
      // ToDo:  if CMC!
      // return LOGIC_OPERATION;
      break;
    }
    return LOGIC_OPERATION;
  }

  void bob_printStatsPeriodical(bool flag);
  void bob_printStats(void);

  bool notify_up(void);

public:
  hmc_bobsim(unsigned id, unsigned num_ports, bool periodPrintStats,
             hmc_cube *cube, hmc_notify *notify, hmc_link *link);
  virtual ~hmc_bobsim(void);

  void clock(void);
  bool bob_feedback(void *packet);
};

#endif /* #ifndef _HMC_BOBSIM_H_ */
