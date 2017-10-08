#ifndef _HMC_TRACE_SQLITE3_H_
#define _HMC_TRACE_SQLITE3_H_

#include <cstdint>
#include <sqlite3.h>
#include "hmc_trace.h"

class hmc_sqlite3 : public hmc_trace_logger {
private:
  bool use_memory;
  const char *dbname;
  sqlite3 *db;
  sqlite3 *filedb;
  sqlite3_stmt *sql;

public:
  /*
   * Let's first dump everything into memory and then store it afterwards to file!
   * This is highly recommended to not be turned off!!
   */
  explicit hmc_sqlite3(const char *dbname = "hmcsim_sqlite3.db", bool use_memory = true);
  ~hmc_sqlite3(void);

  void execute(enum hmc_link_type linkTypeId, unsigned linkIntTypeId,
               uint64_t cycle, uint64_t phyPktAddr,
               int fromCubId, int toCubId,
               int fromId, int toId,
               uint64_t header, uint64_t tail);
};

#endif /* #ifndef _HMC_TRACE_SQLITE3_H_ */
