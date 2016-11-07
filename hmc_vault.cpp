#include <cassert>
#include <cstdint>
#include "hmc_vault.h"
#include "hmc_link.h"

hmc_vault::hmc_vault(unsigned id, hmc_notify *notify, hmc_link *link) :
  id(id),
  link(link)
{
  this->link->set_ilink_notify(id, notify);
}

hmc_vault::~hmc_vault(void)
{

}

#include <iostream>
void hmc_vault::clock(void)
{
  unsigned packetleninbit;
  uint64_t* packet = (uint64_t*)link->get_ilink()->front(&packetleninbit);
  if(packet == nullptr)
    return;
  std::cout << "got packet!!!" << std::endl;
  link->get_ilink()->pop_front();
}
