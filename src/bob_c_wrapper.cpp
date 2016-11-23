#include "../include/bob_c_wrapper.h"
#include "../include/bob_wrapper.h"
#include "../include/bob_transaction.h"
#include "../include/bob_globals.h"

#ifdef HMCSIM_SUPPORT

unsigned BOBSim::NUM_PORTS =1;
int BOBSim::SHOW_SIM_OUTPUT=1;

extern "C" {

BobWrapper* BobNewWrapper(unsigned num_ports, bool periodPrintStats)
{
  BOBSim::NUM_PORTS = num_ports;
  BOBSim::SHOW_SIM_OUTPUT = 0;
  BOBSim::BOBWrapper *bobwrapper = new BOBSim::BOBWrapper();
  bobwrapper->activatedPeriodPrintStates = periodPrintStats;
  return (BobWrapper*)bobwrapper;
}

void BobFreeWrapper(BobWrapper *bobwrapper)
{
  delete (BOBSim::BOBWrapper*)bobwrapper;
}

void BobUpdate(BobWrapper *bobwrapper)
{
  ((BOBSim::BOBWrapper*)bobwrapper)->Update();
}

bool BobSubmitTransaction(BobWrapper *bobwrapper, BobTransaction *bobtransaction, unsigned port)
{
  return ((BOBSim::BOBWrapper*)bobwrapper)->AddTransaction((BOBSim::Transaction*)bobtransaction, port);
}

void BobRegisterCallbacks(BobWrapper *bobwrapper,
            void(*readDone)(unsigned port, uint64_t addr),
            void(*writeDone)(unsigned port, uint64_t addr),
            void(*logicDone)(unsigned port, uint64_t addr))
{
  ((BOBSim::BOBWrapper*)bobwrapper)->RegisterCallbacks(readDone, writeDone, logicDone);
}

void ActPeriodPrintStats(BobWrapper *bobwrapper)
{
  ((BOBSim::BOBWrapper*)bobwrapper)->activatedPeriodPrintStates = true;
}
void BobPrintStats(BobWrapper *bobwrapper)
{
  ((BOBSim::BOBWrapper*)bobwrapper)->PrintStats(true);
}


bool BobIsPortBusy(BobWrapper *bobwrapper, unsigned port)
{
  return ((BOBSim::BOBWrapper*)bobwrapper)->IsPortBusy(port);
}


BobTransaction* BobCreateTransaction(enum TransactionType type, unsigned sizeInBytes, unsigned long addr, void *payload)
{
  return (BobTransaction*)new BOBSim::Transaction((BOBSim::TransactionType)type, sizeInBytes, addr, payload);
}

void BobDeleteTransaction(BobTransaction *bobtransaction)
{
  delete (BOBSim::Transaction*)bobtransaction;
}

void BobAddHMCSIMCallback(BobWrapper *bobwrapper, void *vault, bool (*callback)(void *_vault, void *packet))
{
  ((BOBSim::BOBWrapper*)bobwrapper)->vault = vault;
  ((BOBSim::BOBWrapper*)bobwrapper)->callback = callback;
}


}
#endif /* #ifdef HMCSIM_SUPPORT */
