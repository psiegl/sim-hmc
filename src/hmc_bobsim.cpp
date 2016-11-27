#include "hmc_bobsim.h"
#include "hmc_link.h"
#include "hmc_cube.h"
#include "config.h"

bool callback(void *bobsim, void *packet)
{
  return ((hmc_bobsim*)bobsim)->bob_feedback((char*)packet);
}

int BOBSim::SHOW_SIM_OUTPUT = 1;

hmc_bobsim::hmc_bobsim(unsigned id, unsigned num_ports, bool periodPrintStats,
                       hmc_cube *cube, hmc_notify *notify, hmc_link *link) :
  hmc_notify_cl(),
  hmc_vault(id, cube, &linknotify, link),
  id(id),
  cube(cube),
  linknotify(id, notify, this),
  link(link),
#ifndef ALWAYS_NOTIFY_BOBSIM
  bobnotify_ctr(0),
#endif /* #ifndef ALWAYS_NOTIFY_BOBSIM */
  bobnotify(id, notify, this),
  bobsim(new BOBSim::BOBWrapper(num_ports))
{
  BOBSim::SHOW_SIM_OUTPUT = 0;
  this->bobsim->activatedPeriodPrintStates = periodPrintStats;
  this->bobsim->vault = this;
  this->bobsim->callback = callback;

  this->link->set_ilink_notify(0, &linknotify);
}

hmc_bobsim::~hmc_bobsim(void)
{
  if (!this->id) {
    this->bobsim->PrintStats(true);
  }
  delete this->bobsim;
}


bool hmc_bobsim::bob_feedback(char *packet)
{
  if (this->hmcsim_process_rqst(packet)) {
#ifndef ALWAYS_NOTIFY_BOBSIM
    if (!--this->bobnotify_ctr)
      this->bobnotify.notify_del(0);
#endif /* #ifdef ALWAYS_NOTIFY_BOBSIM */
    return true;
  }
  else {
    this->feedback_cache.push_back(packet);
    return false;
  }
}

void hmc_bobsim::clock(void)
{
#ifndef ALWAYS_NOTIFY_BOBSIM
  if (this->bobnotify_ctr)
#endif /* #ifdef ALWAYS_NOTIFY_BOBSIM */
  {
    if (this->feedback_cache.empty()) {
      this->bobsim->Update();
    }
    else {
      bool update = true;
      for (auto it = this->feedback_cache.begin(); it != this->feedback_cache.end(); ++it) {
        if (!this->bob_feedback(*it)) {
          update = false;
          break;
        }
      }
      if (update) {
        this->bobsim->Update();
      }
    }
  }

  if (this->linknotify.get_notification() && !this->bobsim->IsPortBusy(0 /* port */)) {
    unsigned packetleninbit;
    char *packet = this->link->get_ilink()->front(&packetleninbit);
    if (packet == nullptr)
      return;

    uint64_t header = HMC_PACKET_HEADER(packet);
    uint64_t tail = HMC_PACKET_REQ_TAIL(packet);
    uint64_t addr = HMCSIM_PACKET_REQUEST_GET_ADRS(header);
    hmc_rqst_t cmd = (hmc_rqst_t)HMCSIM_PACKET_REQUEST_GET_CMD(header);

    unsigned rqstlen;
    enum BOBSim::TransactionType type = this->hmc_determineTransactionType(cmd, &rqstlen);
    BOBSim::Transaction *bobtrans = new BOBSim::Transaction(type, rqstlen, 0, packet);
    unsigned long gl_bank = this->cube->HMCSIM_UTIL_DECODE_BANK(addr);
    bobtrans->bank = gl_bank % HMC_NUM_BANKS_PER_RANK;
    bobtrans->rank = (gl_bank - bobtrans->bank) / HMC_NUM_BANKS_PER_RANK;
    this->cube->HMC_UTIL_DECODE_COL_AND_ROW(addr, &bobtrans->col, &bobtrans->row);
    bobtrans->transactionID = HMCSIM_PACKET_REQUEST_GET_SEQ(tail);

    if (!this->bobsim->AddTransaction(bobtrans, 0 /* port */)) {
      exit(0);
    }
#ifdef ALWAYS_NOTIFY_BOBSIM
    else if (!this->bobnotify.get_notification()) {
      // only add forever, if there is something in it
      this->bobnotify.notify_add(0);
    }
#else
    if (!(this->bobnotify_ctr++)) {
      this->bobnotify.notify_add(0);
    }
#endif /* #ifdef ALWAYS_NOTIFY_BOBSIM */
    link->get_ilink()->pop_front();
  }
}

void hmc_bobsim::bob_printStatsPeriodical(bool flag)
{
  this->bobsim->activatedPeriodPrintStates = flag;
}

void hmc_bobsim::bob_printStats(void)
{
  this->bobsim->PrintStats(true);
}

bool hmc_bobsim::notify_up(void)
{
  return (!this->linknotify.get_notification() &&
          !this->bobnotify.get_notification());
}
