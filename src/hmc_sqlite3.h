#ifndef _HMC_SQLITE3_H_
#define _HMC_SQLITE3_H_

#include <sqlite3.h>
#include <stdint.h>

class hmc_sqlite3 {
private:
  sqlite3 *db;

public:
  sqlite3_stmt *sql_rqst;
  sqlite3_stmt *sql_rsp;

  hmc_sqlite3(const char *dbname);
  ~hmc_sqlite3(void);
};

class hmc_trace {
public:
  static void trace_rqst(uint64_t cycle, uint64_t phyPktAddr, unsigned pktTag,
                         uint64_t pktAddr, unsigned typeId, int fromId, int toId);
  static void trace_rsp(uint64_t cycle, uint64_t phyPktAddr, unsigned pktTag,
                        unsigned typeId, int fromId, int toId);
};


#endif /* #ifndef _HMC_SQLITE3_H_ */
