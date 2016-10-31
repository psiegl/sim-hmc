#include "hmc_queue.h"

template <typename T>
hmc_queue<T>::hmc_queue(void) :
  id( -1 ),
  cl( NULL ),
  add( NULL ),
  del( NULL ),
  bitoccupation( 0 ),
  bitoccupationmax( 0 ),
  bitwidth( 0 )
{
}

template <typename T>
hmc_queue<T>::~hmc_queue(void)
{
}

template <typename T>
void hmc_queue<T>::set_notify(unsigned id, T* cl, void (T::*add)(unsigned id), void (T::*del)(unsigned id))
{
  this->id = id;
  this->cl = cl;
  this->add = add;
  this->del = del;
}

template <typename T>
void hmc_queue<T>::re_adjust(unsigned bitwidth, unsigned queuedepth)
{
  this->bitwidth = bitwidth;
  this->bitoccupationmax = bitwidth * queuedepth;
}

template <typename T>
int hmc_queue<T>::has_space(unsigned packetleninbit)
{
  return ! (this->bitoccupation /* + packetleninbit */ < this->bitoccupationmax);
}

template <typename T>
int hmc_queue<T>::push_back(void *packet, unsigned packetleninbit)
{
  if( this->bitoccupation /* + packetleninbit */ < this->bitoccupationmax ) {
    this->bitoccupation += packetleninbit;
    unsigned cycles = packetleninbit / this->bitwidth;
    if( ! cycles )
      cycles = 1;
    this->queue.push(std::make_tuple(packet, cycles, packetleninbit));
    if( cl != NULL )
      (this->cl->*this->add)(this->id);
    return 0;
  }
  return -1;
}

template <typename T>
void* hmc_queue<T>::front(void)
{
  std::tuple<void*, unsigned, unsigned> & front = this->queue.front();
  unsigned cyclestowait = std::get<1>(front);
  return ( ! cyclestowait) ? std::get<0>(front) : NULL;
}

template <typename T>
void* hmc_queue<T>::pop_front(void)
{
  std::tuple<void*, unsigned, unsigned> & front = this->queue.front();
  unsigned cyclestowait = std::get<1>(front);
  if(!cyclestowait)
  {
    this->queue.pop();
    if( cl != NULL )
      (this->cl->*this->del)(this->id);
    return std::get<0>(front);
  }
  else
  {
    std::get<1>(front) -= 1;
    return NULL;
  }
}
