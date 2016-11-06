#ifndef _HMC_ROUTE_H_
#define _HMC_ROUTE_H_

#include "hmc_queue.h"

template <typename T>
class hmc_route {

public:
  hmc_route(void);
  ~hmc_route(void);

  virtual hmc_queue<T>* decode_queue(void *packet) = 0;

  int ship_packet(void *packet, unsigned packetleninbit);

};

#include "hmc_route.tcc"





#endif /* #ifndef _HMC_ROUTE_H_ */
