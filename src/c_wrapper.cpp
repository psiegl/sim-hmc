#include "c_wrapper.h"
#include "BOBWrapper.h"
#include "Transaction.h"
#include "Globals.h"

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




BobTransaction* BobCreateTransaction(enum TransactionType type, unsigned size, unsigned long addr, void *payload)
{
  return (BobTransaction*)new BOBSim::Transaction((BOBSim::TransactionType)type, size, addr);
}

void BobDeleteTransaction(BobTransaction *bobtransaction)
{
  delete (BOBSim::Transaction*)bobtransaction;
}

}
