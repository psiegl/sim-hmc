#ifndef _HMC_QUEUE_H_
#define _HMC_QUEUE_H_

#include <queue>
#include <cstdint>
#include <tuple>

// tuple( packetptr, amount of cycles, totalsizeinbits );
template <typename T>
class hmc_queue {
private:
  unsigned id;
  T* cl;
  void (T::*add)(unsigned id);
  void (T::*del)(unsigned id);

  unsigned bitoccupation;
  unsigned bitoccupationmax;
  unsigned bitwidth;

  std::queue< std::tuple<void*, unsigned, unsigned> > queue;

public:
  hmc_queue(void);
  ~hmc_queue(void);

  void set_notify(unsigned id, T* cl, void (T::*add)(unsigned id), void (T::*del)(unsigned id) );
  void re_adjust(unsigned bitwidth, unsigned queuedepth);

  int has_space(unsigned packetleninbit);
  int push_back(void *packet, unsigned packetleninbit);
  void* front(void);
  void* pop_front(void);
};

#include "hmc_queue.tcc"

#endif /* #ifndef _HMC_QUEUE_H_ */
