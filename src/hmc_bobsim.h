#ifndef _HMC_BOBSIM_H_
#define _HMC_BOBSIM_H_

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
    case CMC04:
    case CMC05:
    case CMC06:
    case CMC07:
    case CMC20:
    case CMC21:
    case CMC22:
    case CMC23:
    case CMC32:
    case CMC36:
    case CMC37:
    case CMC38:
    case CMC39:
    case CMC41:
    case CMC42:
    case CMC43:
    case CMC44:
    case CMC45:
    case CMC46:
    case CMC47:
    case CMC56:
    case CMC57:
    case CMC58:
    case CMC59:
    case CMC60:
    case CMC61:
    case CMC62:
    case CMC63:
    case CMC69:
    case CMC70:
    case CMC71:
    case CMC72:
    case CMC73:
    case CMC74:
    case CMC75:
    case CMC76:
    case CMC77:
    case CMC78:
    case CMC85:
    case CMC86:
    case CMC87:
    case CMC88:
    case CMC89:
    case CMC90:
    case CMC91:
    case CMC92:
    case CMC93:
    case CMC94:
    case CMC102:
    case CMC103:
    case CMC107:
    case CMC108:
    case CMC109:
    case CMC110:
    case CMC111:
    case CMC112:
    case CMC113:
    case CMC114:
    case CMC115:
    case CMC116:
    case CMC117:
    case CMC118:
    case CMC120:
    case CMC121:
    case CMC122:
    case CMC123:
    case CMC124:
    case CMC125:
    case CMC126:
    case CMC127:
      return LOGIC_OPERATION;
      break;
    default:
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
