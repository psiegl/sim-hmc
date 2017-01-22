#ifndef _HMC_CUBE_H_
#define _HMC_CUBE_H_

#include <vector>
#include "hmc_macros.h"
#include "hmc_route.h"
#include "hmc_notify.h"
#include "hmc_register.h"
#include "hmc_connection.h"

class hmc_quad;
class hmc_link;

class hmc_cube : public hmc_route,
                 private hmc_notify_cl,
                 public hmc_register {
private:
  unsigned id;

  hmc_notify quad_notify;
  std::vector<hmc_quad*> quads;
  hmc_notify conn_notify;
  hmc_conn* conn;

  bool notify_up(void);

public:
  hmc_cube(unsigned id, hmc_notify *notify,
           unsigned quadbus_bitwidth, float quadbus_bitrate,
           unsigned capacity,
           std::map<unsigned, hmc_cube*>* cubes, unsigned numcubes, uint64_t *clk);
  virtual ~hmc_cube(void);

  ALWAYS_INLINE unsigned get_id(void)
  {
    return this->id;
  }

  ALWAYS_INLINE hmc_conn_part* get_conn(unsigned id)
  {
    return this->conn->get_conn(id);
  }

  void clock(void);
};


#endif /* #ifndef _HMC_CUBE_H_ */
