#include <cassert>
#include "crossbar.h"
#include "hmc_queue.h"

template <typename T>
hmc_xbar<T>::hmc_xbar(unsigned id) :
  id( id ),
  cl( NULL ),
  add( NULL ),
  del( NULL ),
  ext_link_clock( false ),
  xbar_link_clock( 0 ),
  vault_link_clock( 0 )
{
  for(unsigned i = 0; i < HMC_NUM_QUADS; i++)
  {
    this->xbar_link[i].set_ilink_notify(i, this, &hmc_xbar<T>::notify_xbar_add, &hmc_xbar<T>::notify_xbar_del);
  }
  for(unsigned i = 0; i < HMC_NUM_VAULTS / HMC_NUM_QUADS; i++)
  {
    this->vault_link[i].set_ilink_notify(i, this, &hmc_xbar<T>::notify_vault_add, &hmc_xbar<T>::notify_vault_del);
  }
  this->ext_link.set_ilink_notify(0, this, &hmc_xbar<T>::notify_ext_add, &hmc_xbar<T>::notify_ext_del);
}

template <typename T>
hmc_xbar<T>::~hmc_xbar(void)
{
}

template <typename T>
void hmc_xbar<T>::notify_add(unsigned long* clock, unsigned id)
{
  if( ! (*clock & (0x1 << id)))
  {
    *clock |= (0x1 << id);
    if( this->cl != NULL )
      (this->cl->*add)(this->id);
  }
}

template <typename T>
void hmc_xbar<T>::notify_del(unsigned long* clock, unsigned id)
{
  *clock &= ~(0x1 << id);
  if( this->cl != NULL )
    (this->cl->*del)(this->id);
}

template <typename T>
void hmc_xbar<T>::connect_xbar_link(unsigned id, hmc_link< hmc_xbar<T>, hmc_xbar<T> >* remotelink)
{
  this->xbar_link[id].set_olink( remotelink->get_ilink() );
}

template <typename T>
hmc_link< hmc_xbar<T>, hmc_xbar<T> >* hmc_xbar<T>::get_xbar_link(unsigned id)
{
  return &this->xbar_link[id];
}

template <typename T>
void hmc_xbar<T>::connect_vault_link(unsigned id, hmc_link< hmc_xbar<T>, hmc_xbar<T> >* remotelink)
{
  this->vault_link[id].set_olink( remotelink->get_ilink() );
}

template <typename T>
void hmc_xbar<T>::connect_ext_link(unsigned id, hmc_link< hmc_xbar<T>, hmc_xbar<T> >* remotelink)
{
  this->ext_link.set_olink( remotelink->get_ilink() );
}

template <typename T>
void hmc_xbar<T>::re_adjust_xbar_link(unsigned bitwidth, unsigned queuedepth)
{
  for(unsigned i = 0; i < HMC_NUM_QUADS; i++)
  {
    this->xbar_link[i].re_adjust(bitwidth, queuedepth);
  }
}

template <typename T>
void hmc_xbar<T>::re_adjust_vault_link(unsigned bitwidth, unsigned queuedepth)
{
  for(unsigned i = 0; i < HMC_NUM_VAULTS / HMC_NUM_QUADS; i++)
  {
    this->vault_link[i].re_adjust(bitwidth, queuedepth);
  }
}

template <typename T>
void hmc_xbar<T>::re_adjust_ext_link(unsigned bitwidth, unsigned queuedepth)
{
  this->ext_link.re_adjust(bitwidth, queuedepth);
}

template <typename T>
void hmc_xbar<T>::set_notify(T* cl, void (T::*add)(unsigned id), void (T::*del)(unsigned id))
{
  this->cl = cl;
  this->add = add;
  this->del = del;
}

template <typename T>
void hmc_xbar<T>::clock(void)
{
  if(xbar_link_clock)
  {
    unsigned lid = __builtin_ctzl(this->xbar_link_clock); // round robin?
    hmc_queue< hmc_xbar<T> >* queue = this->xbar_link[lid].get_ilink();
    void *packet = queue->front();
    assert( packet );
    // decode .. and route further! if space available!
  }
  if(ext_link_clock)
  {
    hmc_queue< hmc_xbar<T> >* queue = this->ext_link.get_ilink();
    void *packet = queue->front();
    assert( packet );
    // decode .. and route further! if space available!
  }
  if(vault_link_clock)
  {
    unsigned lid = __builtin_ctzl(this->vault_link_clock); // round robin?
    hmc_queue< hmc_xbar<T> >* queue = this->vault_link[lid].get_ilink();
    void *packet = queue->front();
    assert( packet );
    // decode .. and route further! if space available!
  }
}
