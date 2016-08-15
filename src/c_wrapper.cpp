#include "c_wrapper.h"
#include "BOBWrapper.h"
#include "Transaction.h"

extern "C" {

BobWrapper* newBOBWrapper(unsigned num_ports)
{
  BOBSim::NUM_PORTS = num_ports;
  BOBSim::SHOW_SIM_OUTPUT = 0;
  return (BobWrapper*)new BOBSim::BOBWrapper();
}

void freeBOBWrapper(BobWrapper *bobwrapper)
{
  delete (BOBSim::BOBWrapper*)bobwrapper;
}

bool SubmitTransaction(BobWrapper *bobwrapper, BobTransaction *bobtransaction, unsigned port)
{
  return ((BOBSim::BOBWrapper*)bobwrapper)->AddTransaction((BOBSim::Transaction*)bobtransaction, port);
}

void Update(BobWrapper *bobwrapper)
{
  ((BOBSim::BOBWrapper*)bobwrapper)->Update();
}

void PrintStats(BobWrapper *bobwrapper)
{
  ((BOBSim::BOBWrapper*)bobwrapper)->PrintStats(true);
}

BobTransaction* CreateTransaction(TransactionType type, unsigned size, unsigned long addr)
{
  return (BobTransaction*)new BOBSim::Transaction((BOBSim::TransactionType)type, size, addr);
}

void DeleteTransaction(BobTransaction *bobtransaction)
{
  delete (BOBSim::Transaction*)bobtransaction;
}

}
