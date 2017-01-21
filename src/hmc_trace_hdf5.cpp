#include <iostream>
#include "hmc_trace_hdf5.h"

#define RANK 1

hmc_hdf5::hmc_hdf5(const char *filename)
{
  hid_t file = H5Fcreate(filename, H5F_ACC_TRUNC, H5P_DEFAULT, H5P_DEFAULT);
//  unsigned dims[RANK] = 5;
//  hid_t dataspace = H5Screate_simple(RANK, dims, NULL);

/*
                           "INSERT INTO hmcsim (linkTypeId, linkIntTypeId, cycle, phyPktAddr, fromId, toId, header, tail)" \
                           "values ($1, $2, $3, $4, $5, $6, $7, $8);");

    const char *s_sql = "DROP TABLE IF EXISTS hmcsim CASCADE;" \
                        "CREATE TABLE IF NOT EXISTS hmcsim (" \
                        "linkTypeId     INT," \
                        "linkIntTypeId  INT," \
                        "cycle          BIGINT," \
                        "phyPktAddr     BIGINT," \
                        "fromId         INT," \
                        "toId           INT," \
                        "header         BIGINT," \
                        "tail           BIGINT );"; // we always start with a fresh table ... user needs to back it up
 */
}

hmc_hdf5::~hmc_hdf5(void)
{
}

void hmc_hdf5::execute(unsigned linkTypeId, unsigned linkIntTypeId,
                       uint64_t cycle, uint64_t phyPktAddr,
                       int fromId, int toId,
                       uint64_t header, uint64_t tail)
{
}
