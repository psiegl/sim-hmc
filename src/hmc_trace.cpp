#include <iostream>
#include "hmc_trace.h"
#if defined(HMC_LOGGING_SQLITE3)
# include "hmc_trace_sqlite3.h"
#elif defined(HMC_LOGGING_POSTGRESQL)
# include "hmc_trace_postgresql.h"
#elif defined(HMC_LOGGING_STDOUT)
# include "hmc_trace_stdout.h"
# endif

static hmc_trace_logger *logger = nullptr;

void hmc_trace::trace_setup(void)
{
#if defined(HMC_LOGGING_SQLITE3)
  if (!logger) {
    const char *dbname;
    if (!(dbname = getenv("HMCSIM_TRACE_DBFILE"))) {
      std::cout << "WARNING: please define env variable: " \
        "HMCSIM_TRACE_DBFILE" << std::endl;
      std::cout << "         will try default param." << std::endl;
      logger = new hmc_sqlite3();
    }
    else
      logger = new hmc_sqlite3(dbname);
  }
#elif defined(HMC_LOGGING_POSTGRESQL)
  if (!logger) {
    const char *dbname, *dbuser, *dbpassword, *dbaddr, *dbport;
    if (!(dbname = getenv("HMCSIM_TRACE_DBNAME"))
        || !(dbuser = getenv("HMCSIM_TRACE_DBUSER"))
        || !(dbpassword = getenv("HMCSIM_TRACE_DBPASSWD"))
        || !(dbaddr = getenv("HMCSIM_TRACE_DBADDR"))
        || !(dbport = getenv("HMCSIM_TRACE_DBPORT"))) {
      std::cout << "WARNING: please define all env variables: " \
        "HMCSIM_TRACE_DBNAME, " \
        "HMCSIM_TRACE_DBUSER, " \
        "HMCSIM_TRACE_DBPASSWD, " \
        "HMCSIM_TRACE_DBADDR, " \
        "HMCSIM_TRACE_DBPORT" << std::endl;
      std::cout << "         will try default params." << std::endl;
      logger = new hmc_postgresql();
    }
    else
      logger = new hmc_postgresql(dbname, dbuser, dbpassword, dbaddr, dbport);
  }
#elif defined(HMC_LOGGING_STDOUT)
  logger = new hmc_trace_stdout();
#endif
}

void hmc_trace::trace_cleanup(void)
{
  if (logger)
    delete logger;
}

void hmc_trace::trace_in_rqst(uint64_t cycle, uint64_t phyPktAddr,
                              unsigned typeId, int fromId, int toId,
                              uint64_t header, uint64_t tail)
{
  logger->execute(typeId, 0x0, cycle, phyPktAddr, fromId, toId, header, tail);
}

void hmc_trace::trace_in_rsp(uint64_t cycle, uint64_t phyPktAddr,
                             unsigned typeId, int fromId, int toId,
                             uint64_t header, uint64_t tail)
{
  logger->execute(typeId, 0x1, cycle, phyPktAddr, fromId, toId, header, tail);
}

void hmc_trace::trace_out_rqst(uint64_t cycle, uint64_t phyPktAddr,
                               unsigned typeId, int fromId, int toId,
                               uint64_t header, uint64_t tail)
{
  logger->execute(typeId, 0x2, cycle, phyPktAddr, fromId, toId, header, tail);
}

void hmc_trace::trace_out_rsp(uint64_t cycle, uint64_t phyPktAddr,
                              unsigned typeId, int fromId, int toId,
                              uint64_t header, uint64_t tail)
{
  logger->execute(typeId, 0x3, cycle, phyPktAddr, fromId, toId, header, tail);
}
