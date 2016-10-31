#include "hmc_link.h"

template <typename Ti, typename To>
hmc_link<Ti, To>::hmc_link(void) :
  o( NULL )
{
}

template <typename Ti, typename To>
hmc_link<Ti, To>::~hmc_link(void)
{
}

template <typename Ti, typename To>
void hmc_link<Ti, To>::re_adjust(unsigned bitwidth, unsigned queuedepth)
{
  this->i.re_adjust(bitwidth, queuedepth);
}

template <typename Ti, typename To>
void hmc_link<Ti, To>::set_ilink_notify(unsigned id, Ti* cl, void (Ti::*add)(unsigned id), void (Ti::*del)(unsigned id))
{
  this->i.set_notify(id, cl, add, del);
}

template <typename Ti, typename To>
void hmc_link<Ti, To>::set_olink(hmc_queue<To>* olink)
{
  this->o = olink;
}

template <typename Ti, typename To>
hmc_queue<Ti>* hmc_link<Ti, To>::get_ilink(void)
{
  return &this->i;
}


template <typename Ti, typename To>
hmc_queue<To>* hmc_link<Ti, To>::get_olink(void)
{
  return this->o;
}

