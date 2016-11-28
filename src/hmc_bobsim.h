#ifndef _HMC_BOBSIM_H_
#define _HMC_BOBSIM_H_

#include <list>
#include "hmc_notify.h"
#include "hmc_macros.h"
#include "hmc_vault.h"
#include "../extern/bobsim/include/bob_transaction.h"
#include "../extern/bobsim/include/bob_wrapper.h"

class hmc_link;
class hmc_cube;

#define ALWAYS_NOTIFY_BOBSIM 1

class hmc_bobsim : private hmc_notify_cl, private hmc_vault {
private:
  unsigned id;
  unsigned quadId;
  hmc_cube *cube;

  hmc_notify linknotify;
  hmc_link *link;

#ifndef ALWAYS_NOTIFY_BOBSIM
  unsigned bobnotify_ctr;
#endif /* #ifndef ALWAYS_NOTIFY_BOBSIM */
  hmc_notify bobnotify;
  BOBSim::BOBWrapper *bobsim;

  std::list<char*>feedback_cache;

  enum BOBSim::TransactionType hmc_determineTransactionType(hmc_rqst_t cmd, unsigned *rqstlen)
  {
    switch (cmd) {
    case RD16:
      *rqstlen = 16;
      return BOBSim::DATA_READ;
    case RD32:
      *rqstlen = 32;
      return BOBSim::DATA_READ;
    case RD48:
      *rqstlen = 48;
      return BOBSim::DATA_READ;
    case RD64:
      *rqstlen = 64;
      return BOBSim::DATA_READ;
    case RD80:
      *rqstlen = 80;
      return BOBSim::DATA_READ;
    case RD96:
      *rqstlen = 96;
      return BOBSim::DATA_READ;
    case RD112:
      *rqstlen = 112;
      return BOBSim::DATA_READ;
    case RD128:
      *rqstlen = 128;
      return BOBSim::DATA_READ;
    case RD256:
      *rqstlen = 256;
      return BOBSim::DATA_READ;
    case MD_RD:
      *rqstlen = 8;
      return BOBSim::DATA_READ;
    case FLOW_NULL:
    case PRET:
    case TRET:
    case IRTRY:
    case INC8:
    case P_INC8:
      *rqstlen = 8;
      return BOBSim::LOGIC_OPERATION;
    case WR16:
      *rqstlen = 16;
      return BOBSim::DATA_WRITE;
    case MD_WR:
    case BWR:
      *rqstlen = 8;
      return BOBSim::DATA_WRITE;
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
      return BOBSim::LOGIC_OPERATION;
    case BWR8R:
      return BOBSim::DATA_WRITE;
    case SWAP16:
      return BOBSim::LOGIC_OPERATION;
    case P_WR16:
    case P_BWR:
      return BOBSim::DATA_WRITE;
    case P_2ADD8:
    case P_ADD16:
      return BOBSim::LOGIC_OPERATION;
    case WR32:
    case P_WR32:
      *rqstlen = 32;
      return BOBSim::DATA_WRITE;
    case WR48:
    case P_WR48:
      *rqstlen = 48;
      return BOBSim::DATA_WRITE;
    case WR64:
    case P_WR64:
      *rqstlen = 64;
      return BOBSim::DATA_WRITE;
    case WR80:
    case P_WR80:
      *rqstlen = 80;
      return BOBSim::DATA_WRITE;
    case WR96:
    case P_WR96:
      *rqstlen = 96;
      return BOBSim::DATA_WRITE;
    case WR112:
    case P_WR112:
      *rqstlen = 112;
      return BOBSim::DATA_WRITE;
    case WR128:
    case P_WR128:
      *rqstlen = 128;
      return BOBSim::DATA_WRITE;
    case WR256:
    case P_WR256:
      *rqstlen = 256;
      return BOBSim::DATA_WRITE;
      break;
    default:
      // ToDo:  if CMC!
      // return LOGIC_OPERATION;
      break;
    }
    return BOBSim::LOGIC_OPERATION;
  }

  void bob_printStatsPeriodical(bool flag);
  void bob_printStats(void);

  bool notify_up(void);

public:
  hmc_bobsim(unsigned id, unsigned quadId, unsigned num_ports, unsigned num_ranks, bool periodPrintStats,
             hmc_cube *cube, hmc_notify *notify, hmc_link *link);
  virtual ~hmc_bobsim(void);

  void clock(void);
  bool bob_feedback(char *packet);
};

#endif /* #ifndef _HMC_BOBSIM_H_ */
