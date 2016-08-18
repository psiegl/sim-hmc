#ifdef HMCSIM_SUPPORT
#include "bob_c_wrapper.h"
#include "bob_wrapper.h"
#include "bob_transaction.h"
#include "bob_globals.h"

unsigned BOBSim::NUM_PORTS =1;
int BOBSim::SHOW_SIM_OUTPUT=1;

extern "C" {

BobWrapper* BobNewWrapper(unsigned num_ports)
{
  BOBSim::NUM_PORTS = num_ports;
  BOBSim::SHOW_SIM_OUTPUT = 0;
  return (BobWrapper*)new BOBSim::BOBWrapper();
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

void BobPrintStats(BobWrapper *bobwrapper)
{
  ((BOBSim::BOBWrapper*)bobwrapper)->PrintStats(true);
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
