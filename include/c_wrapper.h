#ifndef __C_WRAPPER_H__
#define __C_WRAPPER_H__

#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

enum TransactionType
{
  DATA_READ       = 0x1,
  DATA_WRITE      = 0x2,
  RETURN_DATA     = 0x3,
  LOGIC_OPERATION = 0x4,
  LOGIC_RESPONSE  = 0x5
};

typedef struct BobWrapper BobWrapper;
typedef struct BobTransaction BobTransaction;

BobWrapper* BobNewWrapper(unsigned num_ports);
void BobFreeWrapper(BobWrapper *bobwrapper);

void BobUpdate(BobWrapper *bobwrapper);
bool BobSubmitTransaction(BobWrapper *bobwrapper, BobTransaction *bobtransaction, unsigned port);
void BobRegisterCallbacks(BobWrapper *bobwrapper,
            void(*readDone)(unsigned port, uint64_t addr),
            void(*writeDone)(unsigned port, uint64_t addr),
            void(*logicDone)(unsigned port, uint64_t addr));
void BobPrintStats(BobWrapper *bobwrapper);


BobTransaction* BobCreateTransaction(TransactionType type, unsigned size, unsigned long addr, void *payload);
void BobDeleteTransaction(BobTransaction *bobtransaction);

#ifdef __cplusplus
}
#endif
#endif /* __C_WRAPPER_H__ */
