#ifndef _HMC_BOBSIM_H_
#define _HMC_BOBSIM_H_

#include <list>
#include "hmc_notify.h"
#include "hmc_macros.h"
#include "hmc_vault.h"
#include "../extern/bobsim/include/bob_transaction.h"
#include "../extern/bobsim/include/bob_wrapper.h"
#include "hmc_module.h"

class hmc_link;
class hmc_cube;

class hmc_bobsim : private hmc_notify_cl, public hmc_module {
private:
  unsigned id;
#ifndef BOBSIM_NO_LOG
  unsigned quadId;
#endif /* #ifndef BOBSIM_NO_LOG */
  hmc_cube *cube;

  hmc_link *link;
  hmc_notify linknotify;

  hmc_vault vault;

#ifdef HMC_USES_NOTIFY
  unsigned bobnotify_ctr;
#endif /* #ifdef HMC_USES_NOTIFY */
  hmc_notify bobnotify;
  BOBSim::BOBWrapper *bobsim;

  std::list<char*>feedback_cache;

  enum BOBSim::TransactionType hmc_determineTransactionType(hmc_rqst_t cmd)
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
      return BOBSim::DATA_READ;
    case FLOW_NULL:
    case PRET:
    case TRET:
    case IRTRY:
    case INC8:
    case P_INC8:
      return BOBSim::LOGIC_OPERATION;
    case WR16:
    case MD_WR:
    case BWR:
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

  bool notify_up(unsigned id);

public:
  hmc_bobsim(unsigned id, unsigned quadId, unsigned num_ports,
             unsigned num_ranks, bool periodPrintStats,
             hmc_cube *cube, hmc_notify *notify);
  virtual ~hmc_bobsim(void);

  void clock(void);
  bool bob_feedback(char *packet);

  unsigned get_id(void) { return this->id; }
  bool set_link(unsigned linkId, hmc_link *link, hmc_link_type linkType) {
    this->link = link;
    this->vault.set_link(linkId, link, linkType);
    return true;
  }

};

#endif /* #ifndef _HMC_BOBSIM_H_ */
