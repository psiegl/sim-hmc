#include <iostream>
#include "hmc_trace_postgresql.h"

hmc_postgresql::hmc_postgresql(const char *dbname, const char *dbuser, const char *dbpasswd,
                               const char *dbaddr, const char *dbport)
{
  std::string str = std::string("dbname=") + std::string(dbname);
  str += std::string(" user=") + std::string(dbuser);
  str += std::string(" password=") + std::string(dbpasswd);
  str += std::string(" hostaddr=") + std::string(dbaddr);
  str += std::string(" port=") + std::string(dbport);

  this->db_conn = new pqxx::connection(str.c_str());
  if (this->db_conn->is_open()) {
    std::cout << "PSQL: Opened database successfully: " << this->db_conn->dbname() << std::endl;
  }
  else {
    std::cerr << "ERROR: PSQL: Can't open database" << std::endl;
    exit(-1);
  }

  try {
    this->db_conn->prepare("insert",
                           "INSERT INTO hmcsim (linkTypeId, linkIntTypeId, cycle, phyPktAddr, fromId, toId, header, tail)" \
                           "values ($1, $2, $3, $4, $5, $6, $7, $8);");
  }
  catch (const std::exception &e) {
    std::cerr << "ERROR: " << e.what() << std::endl;
    exit(-1);
  }

  try {
    const char *s_sql = "DROP TABLE IF EXISTS hmcsim CASCADE;" \
                        "CREATE TABLE IF NOT EXISTS hmcsim (" \
                        "linkTypeId     INT," \
                        "linkIntTypeId  INT," \
                        "cycle          BIGINT," \
                        "phyPktAddr     BIGINT," \
                        "fromId         INT," \
                        "toId           INT," \
                        "header         BIGINT," \
                        "tail           BIGINT );";   // we always start with a fresh table ... user needs to back it up

    this->trans = new pqxx::work(*this->db_conn);
    this->trans->exec(s_sql);
  }
  catch (const std::exception &e) {
    std::cerr << "ERROR: " << e.what() << std::endl;
    exit(-1);
  }

//  pqxx::pipeline pipe(*this->db_conn);
}

hmc_postgresql::~hmc_postgresql(void)
{
  this->trans->commit();
  this->db_conn->disconnect();
  delete this->db_conn;
}

void hmc_postgresql::execute(unsigned linkTypeId, unsigned linkIntTypeId,
                             uint64_t cycle, uint64_t phyPktAddr,
                             int fromId, int toId,
                             uint64_t header, uint64_t tail)
{
  // PostgreSQL does only support signed 64-bit integer. So let's convert the unsigned ones :-)
  int64_t i_cycle = *(int64_t*)&cycle;
  int64_t i_phyPktAddr = *(int64_t*)&phyPktAddr;
  int64_t i_header = *(int64_t*)&header;
  int64_t i_tail = *(int64_t*)&tail;

//  try {
  this->trans->prepared("insert")(linkTypeId)(linkIntTypeId)(i_cycle)(i_phyPktAddr)(fromId)(toId)(i_header)(i_tail).exec();
//  }
//  catch (const std::exception &e) {
//    std::cerr << "ERROR: " << e.what() << std::endl;
//    exit(-1);
//  }
}
