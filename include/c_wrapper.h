#ifndef __C_WRAPPER_H__
#define __C_WRAPPER_H__

#include <stdbool.h>

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

BobWrapper* newBOBWrapper(unsigned num_ports);
void freeBOBWrapper(BobWrapper *bobwrapper);

bool SubmitTransaction(BobWrapper *bobwrapper, BobTransaction *bobtransaction, unsigned port);
void Update(BobWrapper *bobwrapper);
void PrintStats(BobWrapper *bobwrapper);

BobTransaction* CreateTransaction(TransactionType type, unsigned size, unsigned long addr);
void DeleteTransaction(BobTransaction *bobtransaction);

#ifdef __cplusplus
}
#endif
#endif /* __C_WRAPPER_H__ */
