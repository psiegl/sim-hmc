#include "c_wrapper.h"
#include "BOBWrapper.h"
#include "Transaction.h"

extern "C" {



BobWrapper* BobNewWrapper(unsigned num_ports,
            void(*readDone)(unsigned port, uint64_t addr),
            void(*writeDone)(unsigned port, uint64_t addr),
            void(*logicDone)(unsigned port, uint64_t addr))
{
  BOBSim::NUM_PORTS = num_ports;
  BOBSim::SHOW_SIM_OUTPUT = 0;
  BOBSim::BOBWrapper *bobwrapper = new BOBSim::BOBWrapper();
  bobwrapper->RegisterCallbacks(readDone, writeDone, logicDone);
  return (BobWrapper*)bobwrapper;
}

void BobFreeWrapper(BobWrapper *bobwrapper)
{
  delete (BOBSim::BOBWrapper*)bobwrapper;
}

bool BobSubmitTransaction(BobWrapper *bobwrapper, BobTransaction *bobtransaction, unsigned port)
{
  return ((BOBSim::BOBWrapper*)bobwrapper)->AddTransaction((BOBSim::Transaction*)bobtransaction, port);
}

void BobUpdate(BobWrapper *bobwrapper)
{
  ((BOBSim::BOBWrapper*)bobwrapper)->Update();
}

void BobPrintStats(BobWrapper *bobwrapper)
{
  ((BOBSim::BOBWrapper*)bobwrapper)->PrintStats(true);
}

BobTransaction* BobCreateTransaction(TransactionType type, unsigned size, unsigned long addr, void *payload)
{
  return (BobTransaction*)new BOBSim::Transaction((BOBSim::TransactionType)type, size, addr);
}

void BobDeleteTransaction(BobTransaction *bobtransaction)
{
  delete (BOBSim::Transaction*)bobtransaction;
}

}
