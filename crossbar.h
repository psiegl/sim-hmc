#ifndef _HMC_XBAR_H_
#define _HMC_XBAR_H_

#include "hmc_link.h"
#include "config.h"

template <typename T>
class hmc_xbar {
private:
  unsigned id;
  T *cl;
  void (T::*add)(unsigned id);
  void (T::*del)(unsigned id);

  unsigned long xbar_link_clock; // round robin?
  hmc_link< hmc_xbar<T>, hmc_xbar<T> > xbar_link[ HMC_NUM_QUADS ];
  unsigned long vault_link_clock; // round robin?
  hmc_link< hmc_xbar<T>, hmc_xbar<T> > vault_link[ HMC_NUM_VAULTS / HMC_NUM_QUADS ]; // ToDo
  unsigned long ext_link_clock;
  hmc_link< hmc_xbar<T>, hmc_xbar<T> > ext_link; // ToDo

  ALWAYS_INLINE void notify_add( unsigned long* clock, unsigned id );
  ALWAYS_INLINE void notify_del( unsigned long* clock, unsigned id );

  void notify_xbar_add(unsigned id) {
    this->notify_add( &xbar_link_clock, id );
  }
  void notify_xbar_del(unsigned id) {
    this->notify_del( &xbar_link_clock, id );
  }

  void notify_vault_add(unsigned id) {
    this->notify_add( &vault_link_clock, id );
  }
  void notify_vault_del(unsigned id) {
    this->notify_del( &vault_link_clock, id );
  }

  void notify_ext_add(unsigned id) {
    this->notify_add( &vault_link_clock, 0 );
  }
  void notify_ext_del(unsigned id) {
    this->notify_del( &vault_link_clock, 0 );
  }

public:
  hmc_xbar(unsigned id);
  ~hmc_xbar(void);

  void connect_xbar_link(unsigned id, hmc_link< hmc_xbar<T>, hmc_xbar<T> >* remotelink);
  hmc_link< hmc_xbar<T>, hmc_xbar<T> > * get_xbar_link(unsigned id);
  
  void connect_vault_link(unsigned id, hmc_link< hmc_xbar<T>, hmc_xbar<T> >* remotelink);
  void connect_ext_link(unsigned id, hmc_link< hmc_xbar<T>, hmc_xbar<T> >* remotelink);

  void re_adjust_xbar_link(unsigned bitwidth, unsigned queuedepth);
  void re_adjust_vault_link(unsigned bitwidth, unsigned queuedepth);
  void re_adjust_ext_link(unsigned bitwidth, unsigned queuedepth);

  void set_notify(T* cl, void (T::*add)(unsigned id), void (T::*del)(unsigned id));

  void clock(void);
};

#include "crossbar.tcc"

#endif /* #ifndef _HMC_XBAR_H_ */
