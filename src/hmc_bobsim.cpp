#include "hmc_bobsim.h"
#include "hmc_link.h"
#include "hmc_cube.h"

bool callback(void *bobsim, void *packet)
{
  return ((hmc_bobsim*)bobsim)->bob_feedback(packet);
}

hmc_bobsim::hmc_bobsim(unsigned id, unsigned num_ports, bool periodPrintStats,
                       hmc_cube *cube, hmc_notify *notify, hmc_link *link) :
  hmc_notify_cl(),
  hmc_vault(id, cube, &linknotify, link),
  id(id),
  cube(cube),
  linknotify(id, notify, this),
  link(link),
  bobnotify_ctr(0),
  bobnotify(id, notify, this),
  bobsim(BobNewWrapper(num_ports, periodPrintStats))
{
  this->link->set_ilink_notify(0, &linknotify);
  BobAddHMCSIMCallback(this->bobsim, this, callback);
}

hmc_bobsim::~hmc_bobsim(void)
{
  if (!this->id) {
    BobPrintStats(this->bobsim);
  }
  BobFreeWrapper(this->bobsim);
}


bool hmc_bobsim::bob_feedback(void *packet)
{
  if (!--this->bobnotify_ctr)
    this->bobnotify.notify_del(0);
  return this->hmcsim_process_rqst(packet);
}

void hmc_bobsim::clock(void)
{
  if (this->bobnotify_ctr) {
    void *packet = BobWillCallback(this->bobsim, 0 /* num_ports */);
    if (packet) {
      uint64_t header = HMC_PACKET_HEADER(packet);
      hmc_rqst_t cmd = (hmc_rqst_t)HMCSIM_PACKET_REQUEST_GET_CMD(header);

      // ToDo: writes are not yet considered!

      bool no_response;
      unsigned rsp_len;
      this->hmcsim_packet_resp_len(cmd, &no_response, &rsp_len);
      unsigned packetleninbit = rsp_len * FLIT_WIDTH;

      hmc_queue *o_queue = this->link->get_olink();
      if (no_response || o_queue->has_space(packetleninbit)) {
        BobUpdate(this->bobsim);
      }
    }
    else{
      BobUpdate(this->bobsim);
    }
  }

  if (this->linknotify.get_notification() && !BobIsPortBusy(this->bobsim, 0 /* port */)) {
    unsigned packetleninbit;
    void *packet = this->link->get_ilink()->front(&packetleninbit);
    if (packet == nullptr)
      return;

    uint64_t header = HMC_PACKET_HEADER(packet);
    uint64_t addr = HMCSIM_PACKET_REQUEST_GET_ADRS(header);
    hmc_rqst_t cmd = (hmc_rqst_t)HMCSIM_PACKET_REQUEST_GET_CMD(header);
    unsigned bank = this->cube->HMCSIM_UTIL_DECODE_BANK(addr);

    enum TransactionType type = this->hmc_determineTransactionType(cmd);
    BobTransaction *bobtrans = BobCreateTransaction(type, packetleninbit, bank, packet);
    if (!BobSubmitTransaction(this->bobsim, bobtrans, 0 /* port */)) {
      exit(0);
    }

    if (!(this->bobnotify_ctr++)) {
      this->bobnotify.notify_add(0);
    }
    link->get_ilink()->pop_front();
  }
}

void hmc_bobsim::bob_printStatsPeriodical(bool flag)
{
  ActPeriodPrintStats(this->bobsim);
}

void hmc_bobsim::bob_printStats(void)
{
  BobPrintStats(this->bobsim);
}

bool hmc_bobsim::notify_up(void)
{
  return(!this->linknotify.get_notification() &&
         !this->bobnotify.get_notification());
}
