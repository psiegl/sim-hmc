#ifndef _HMC_TRACE_SQLITE3_H_
#define _HMC_TRACE_SQLITE3_H_

#include <sqlite3.h>
#include "hmc_trace.h"

class hmc_sqlite3 : public hmc_trace_logger {
private:
  sqlite3 *db;
  sqlite3_stmt *sql;

public:
  explicit hmc_sqlite3(const char *dbname);
  ~hmc_sqlite3(void);

  void execute(unsigned linkTypeId, unsigned linkIntTypeId,
               uint64_t cycle, uint64_t phyPktAddr,
               int fromId, int toId,
               uint64_t header, uint64_t tail);
};

#endif /* #ifndef _HMC_TRACE_SQLITE3_H_ */
