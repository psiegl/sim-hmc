#include "hmc_bobsim.h"
#include "hmc_link.h"
#include "hmc_cube.h"
#include "config.h"
#include "hmc_notify.h"

bool callback(void *bobsim, void *packet)
{
  return ((hmc_bobsim*)bobsim)->bob_feedback((char*)packet);
}

int BOBSim::SHOW_SIM_OUTPUT = 1;

hmc_bobsim::hmc_bobsim(unsigned id, unsigned quadId, unsigned num_ports, unsigned num_ranks, bool periodPrintStats,
                       hmc_cube *cube, hmc_notify *notify) :
  hmc_notify_cl(),
  id(id),
#ifndef BOBSIM_NO_LOG
  quadId(quadId),
#endif /* #ifndef BOBSIM_NO_LOG */
  cube(cube),
  link(nullptr),
  linknotify(id, notify, this),
  vault(id, cube, &linknotify),
#ifdef HMC_USES_NOTIFY
  bobnotify_ctr(0),
#endif /* #ifdef HMC_USES_NOTIFY */
  bobnotify(id, notify, this),
  bobsim(new BOBSim::BOBWrapper(num_ports, num_ranks, num_ranks)) // DEVICE_WIDTH == NUM_RANKS
{
  BOBSim::SHOW_SIM_OUTPUT = 0;
#ifndef BOBSIM_NO_LOG
  this->bobsim->activatedPeriodPrintStates = periodPrintStats;
#endif
  this->bobsim->vault = this;
  this->bobsim->callback = callback;
}

hmc_bobsim::~hmc_bobsim(void)
{
#ifndef BOBSIM_NO_LOG
  if (this->bobnotify.get_notification()) {
    std::cout << "- HMC cube: " << this->cube->get_id() << ", quad: " << this->quadId << ", vault: " << this->id << " -" << std::endl;
    this->bobsim->PrintStats(true);
  }
#endif

  delete this->bobsim;
}


bool hmc_bobsim::bob_feedback(char *packet)
{
  if (this->vault.hmcsim_process_rqst(packet)) {
#ifdef HMC_USES_NOTIFY
    if (this->bobnotify_ctr && !--this->bobnotify_ctr)
      this->bobnotify.notify_del(0);
#endif /* #ifdef HMC_USES_NOTIFY */
    return true;
  }
  else {
    this->feedback_cache.push_back(packet);
//    std::cout << "shiiiiiiiTTTT!" << std::endl;
    return false;
  }
}

void hmc_bobsim::clock(void)
{
#ifdef HMC_USES_NOTIFY
  if (this->bobnotify_ctr || this->bobnotify.get_notification())
#endif /* #ifdef HMC_USES_NOTIFY */
  {
    if (this->feedback_cache.empty()) {
      this->bobsim->Update();
    }
    else {
//      std::cerr << "WARNING: this shouldn't happen!" << std::endl;
//      std::cerr << "This can only happen, if bobsim pushes further responses,";
//      std::cerr << " while this module can't push it further ... " << std::endl;
      bool update = true;
      for (auto it = this->feedback_cache.begin(); it != this->feedback_cache.end(); ++it) {
        if (!this->bob_feedback(*it)) {
          update = false;
          break;
        }
      }
      if (update)
        this->bobsim->Update();
    }

#ifdef HMC_USES_NOTIFY
    // this is the case when there are no return packets, but we just schedule a little bit furter
    if (!this->bobnotify.get_notification()
        && this->bobnotify_ctr
        && !--this->bobnotify_ctr) {
      this->bobnotify.notify_del(0);
    }
#endif /* #ifdef HMC_USES_NOTIFY */
  }

#ifdef HMC_USES_NOTIFY
  if (this->linknotify.get_notification())
#endif /* #ifdef HMC_USES_NOTIFY */
  {
    this->link->clock();

    if (!this->bobsim->IsPortBusy(0 /* port */)) {
      unsigned packetleninbit;
      char *packet = this->link->get_rx_fifo_out()->front(&packetleninbit);
      if (packet == nullptr)
        return;

      uint64_t header = HMC_PACKET_HEADER(packet);
      uint64_t tail = HMC_PACKET_REQ_TAIL(packet);
      uint64_t addr = HMCSIM_PACKET_REQUEST_GET_ADRS(header);
      hmc_rqst_t cmd = (hmc_rqst_t)HMCSIM_PACKET_REQUEST_GET_CMD(header);
#ifdef HMC_USES_NOTIFY
      bool has_response = this->vault.pkt_has_response(cmd);
#endif /* #ifdef HMC_USES_NOTIFY */

      enum BOBSim::TransactionType type = this->hmc_determineTransactionType(cmd);

      unsigned rsp_lenInFlits;
      this->vault.hmcsim_packet_resp_len(cmd, &rsp_lenInFlits);

      unsigned flits = HMCSIM_PACKET_REQUEST_GET_LNG(header);
      rsp_lenInFlits -= (rsp_lenInFlits > 0); // netto! without overhead!
      flits -= (flits > 0); // netto! without overhead!, because BOBSim accounts by itself for the packet overhead!
      BOBSim::Transaction *bobtrans = new BOBSim::Transaction(type, 0 /* addr */, flits, rsp_lenInFlits, packet);

      unsigned long gl_bank = this->cube->HMCSIM_UTIL_DECODE_BANK(addr);
      bobtrans->bank = gl_bank % HMC_NUM_BANKS_PER_RANK;
      bobtrans->rank = (gl_bank - bobtrans->bank) / HMC_NUM_BANKS_PER_RANK;
      this->cube->HMC_UTIL_DECODE_COL_AND_ROW(addr, &bobtrans->col, &bobtrans->row);
      bobtrans->transactionID = HMCSIM_PACKET_REQUEST_GET_SEQ(tail);

      // adding will always work, since we checked that upfront! (IsPortBusy)
      this->bobsim->AddTransaction(bobtrans, 0 /* port */);

#ifdef HMC_USES_NOTIFY
      // the deal is the following:
      // some request (if implemented, currently not) will not return with a packet!
      // in any case we have to set the notify bit
      // in the case it does not return, we will cycle the the bobsim a little bit further ...
      this->bobnotify.notify_add(0);
      if (!has_response) {
        this->bobnotify_ctr = 100;
      }
#endif /* #ifdef HMC_USES_NOTIFY */

      this->link->get_rx_fifo_out()->pop_front();
    }
  }
}

void hmc_bobsim::bob_printStatsPeriodical(bool flag)
{
#ifndef BOBSIM_NO_LOG
  this->bobsim->activatedPeriodPrintStates = flag;
#endif
}

void hmc_bobsim::bob_printStats(void)
{
#ifndef BOBSIM_NO_LOG
  this->bobsim->PrintStats(true);
#endif
}

bool hmc_bobsim::notify_up(unsigned id)
{
#ifdef HMC_USES_NOTIFY
  return (!this->linknotify.get_notification() &&
          !this->bobnotify.get_notification());
#else
  return true;
#endif /* #ifdef HMC_USES_NOTIFY */
}
