#include <iostream>
#include <fstream>
#include <cstdio>
#include "hmc_trace_sqlite3.h"

// http://www.wassen.net/sqlite-c.html
hmc_sqlite3::hmc_sqlite3(const char *dbname, bool use_memory) :
  use_memory(use_memory),
  dbname(dbname)
{
  if (dbname == nullptr) {
    std::cerr << "HMC_SQLITE3: ERROR: dbname not set!" << std::endl;
    exit(-1);
  }

  // check if file exists. If so, delete
  std::ifstream ifile(dbname);
  if (ifile)
    remove(dbname);

  if (this->use_memory) {
    if (sqlite3_open(":memory:", &this->db)) {
      std::cerr << "ERROR: couldn't open database '" << dbname << "'!" << std::endl;
      exit(-1);
    }
  }
  else {
    if (sqlite3_open_v2(dbname, &this->db, SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE, nullptr)) {
      std::cerr << "ERROR: couldn't open database '" << dbname << "'!" << std::endl;
      exit(-1);
    }
  }

  sqlite3_stmt *query;
  const char *squery = "CREATE TABLE hmcsim_stat (linkTypeId INTEGER, linkIntTypeId INTEGER, cycle INTEGER, physicalPktAddr INTEGER, fromCubId INTEGER, toCubId INTEGER, fromID INTEGER, toId INTEGER, pktHeader INTEGER, pktTail INTEGER);";
  sqlite3_prepare_v2(this->db, squery, -1, &query, nullptr);
  if (sqlite3_step(query) != SQLITE_DONE) {
    std::cerr << "ERROR: create table SQL failed!" << std::endl;
    exit(-1);
  }
  sqlite3_finalize(query);

  const char *s_sql = "INSERT INTO hmcsim_stat (linkTypeId, linkIntTypeId, cycle, physicalPktAddr, fromCubId, toCubId, fromId, toId, pktHeader, pktTail) values (?1, ?2, ?3, ?4, ?5, ?6, ?7, ?8, ?9, ?10);";
  sqlite3_prepare_v2(this->db, s_sql, -1, &this->sql, nullptr);
}

hmc_sqlite3::~hmc_sqlite3(void)
{
  sqlite3_finalize(this->sql);

  if (this->use_memory) {
    if (sqlite3_open_v2(this->dbname, &this->filedb, SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE, nullptr)) {
      std::cerr << "ERROR: couldn't open database '" << dbname << "'!" << std::endl;
      exit(-1);
    }
    sqlite3 *pFrom = this->db;
    sqlite3 *pTo = this->filedb;

    // https://www.sqlite.org/backup.html
    sqlite3_backup *pBackup = sqlite3_backup_init(pTo, "main", pFrom, "main");
    if (pBackup) {
      sqlite3_backup_step(pBackup, -1);
      sqlite3_backup_finish(pBackup);
    }
    if (sqlite3_errcode(pTo))
      std::cerr << "ERROR: couldn't dump data into db-file!" << std::endl;

    sqlite3_close(this->filedb);
  }

  sqlite3_close(this->db);
}

void hmc_sqlite3::execute(enum hmc_link_type linkTypeId, unsigned linkIntTypeId,
                          uint64_t cycle, uint64_t phyPktAddr,
                          int fromCubId, int toCubId,
                          int fromId, int toId,
                          uint64_t header, uint64_t tail)
{
  sqlite3_bind_int(this->sql, 1, (int)linkTypeId);
  sqlite3_bind_int(this->sql, 2, (int)linkIntTypeId);
  sqlite3_bind_int64(this->sql, 3, cycle);
  sqlite3_bind_int64(this->sql, 4, phyPktAddr);
  sqlite3_bind_int(this->sql, 5, fromCubId);
  sqlite3_bind_int(this->sql, 6, toCubId);
  sqlite3_bind_int(this->sql, 7, fromId);
  sqlite3_bind_int(this->sql, 8, toId);
  sqlite3_bind_int64(this->sql, 9, header);
  sqlite3_bind_int64(this->sql, 10, tail);
  if (sqlite3_step(this->sql) != SQLITE_DONE) {
    std::cerr << "ERROR: insert SQL failed!" << std::endl;
    exit(-1);
  }
  sqlite3_reset(this->sql); // clears the binding
}
