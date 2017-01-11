#include <iostream>
#include <fstream>
#include <cstdio>
#include "hmc_sqlite3.h"

static hmc_sqlite3 *hmc_sqlite3_data = nullptr;
hmc_sqlite3 **hmc_trace_init = (hmc_sqlite3**)&hmc_sqlite3_data;

// http://www.wassen.net/sqlite-c.html
hmc_sqlite3::hmc_sqlite3(const char *dbname)
{
  std::cout << "dbname : " << dbname << std::endl;

  // check if file exists. If so, delete
  std::ifstream ifile(dbname);
  if (ifile) {
    remove(dbname);
  }

  if (sqlite3_open_v2(dbname, &this->db, SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE, nullptr)) {
    std::cerr << "ERROR: couldn't open database '" << dbname << "'!" << std::endl;
    exit(-1);
  }

  sqlite3_stmt *query;
  const char *squery = "CREATE TABLE hmcsim_stat (cycle INTEGER, pktTag INTEGER, pktAddr INTEGER, typeId INTEGER, fromID INTEGER, toId INTEGER);";
  sqlite3_prepare_v2(this->db, squery, -1, &query, nullptr);
  if (sqlite3_step(query) != SQLITE_DONE) {
    std::cerr << "ERROR: create table SQL failed!" << std::endl;
    exit(-1);
  }
  sqlite3_finalize(query);

  const char *s_sql_rqst = "INSERT INTO hmcsim_stat (cycle, pktTag, pktAddr, typeId, fromId, toId) values (?1, ?2, ?3, ?4, ?5, ?6);";
  sqlite3_prepare_v2(this->db, s_sql_rqst, -1, &this->sql_rqst, nullptr);

  const char *s_sql_rsp = "INSERT INTO hmcsim_stat (cycle, pktTag, typeId, fromId, toId) values (?1, ?2, ?3, ?4, ?5);";
  sqlite3_prepare_v2(this->db, s_sql_rsp, -1, &this->sql_rsp, nullptr);
}

hmc_sqlite3::~hmc_sqlite3(void)
{
  sqlite3_finalize(this->sql_rqst);
  sqlite3_finalize(this->sql_rsp);
  sqlite3_close(this->db);
  std::cout << "closed db!" << std::endl;
}

void hmc_trace::trace_rqst(uint64_t cycle, unsigned pktTag, uint64_t pktAddr, unsigned typeId, unsigned fromId, unsigned toId)
{
  sqlite3_bind_int64(hmc_sqlite3_data->sql_rqst, 1, cycle);
  sqlite3_bind_int(hmc_sqlite3_data->sql_rqst, 2, pktTag);
  sqlite3_bind_int64(hmc_sqlite3_data->sql_rqst, 3, pktAddr);
  sqlite3_bind_int(hmc_sqlite3_data->sql_rqst, 4, typeId);
  sqlite3_bind_int(hmc_sqlite3_data->sql_rqst, 5, fromId);
  sqlite3_bind_int(hmc_sqlite3_data->sql_rqst, 6, toId);
  if (sqlite3_step(hmc_sqlite3_data->sql_rqst) != SQLITE_DONE) {
    std::cerr << "ERROR: insert rqst SQL failed!" << std::endl;
    exit(-1);
  }
  sqlite3_reset(hmc_sqlite3_data->sql_rqst); // clears the binding
}

void hmc_trace::trace_rsp(uint64_t cycle, unsigned pktTag, unsigned typeId, unsigned fromId, unsigned toId)
{
  sqlite3_bind_int64(hmc_sqlite3_data->sql_rsp, 1, cycle);
  sqlite3_bind_int(hmc_sqlite3_data->sql_rsp, 2, pktTag);
  sqlite3_bind_int(hmc_sqlite3_data->sql_rsp, 3, typeId);
  sqlite3_bind_int(hmc_sqlite3_data->sql_rsp, 4, fromId);
  sqlite3_bind_int(hmc_sqlite3_data->sql_rsp, 5, toId);
  if (sqlite3_step(hmc_sqlite3_data->sql_rsp) != SQLITE_DONE) {
    std::cerr << "ERROR: insert rsp SQL failed!" << std::endl;
    exit(-1);
  }
  sqlite3_reset(hmc_sqlite3_data->sql_rsp); // clears the binding
}
