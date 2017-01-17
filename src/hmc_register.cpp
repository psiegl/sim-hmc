#include <iostream>
#include <cstdint>
#include "config.h"
#include "hmc_cube.h"
#include "hmc_register.h"

hmc_register::hmc_register(hmc_cube *cube, unsigned capacity) :
  hmc_decode(),
  cube(cube)
{
  for (unsigned i = 0; i < HMC_NUM_REGS; i++)
    this->regs[i] = 0x0;

  // reset all the registers
  for (unsigned i = 0; i < (unsigned)elemsof(this->hmcsim_decode); i++) {
    struct hmcsim_reg_decode_fields_t *field;
    if (!this->hmcsim_get_decode_field(this->hmcsim_decode[i].name, &field)) {
      std::cerr << "ERROR: No mapping found!" << std::endl;
      continue;
    }
    this->hmcsim_reg_value_reset(this->hmcsim_decode[i].reg, this->hmcsim_decode[i].name, field->reset_value);
  }

  // set specific registers as of supplied configuration
  this->hmcsim_reg_value_reset(HMC_REG_FEAT, HMC_REG_FEAT__CUBE_SIZE, (unsigned)log2(capacity) - 1);
  unsigned num_banks = 2 * capacity;
  this->hmcsim_reg_value_reset(HMC_REG_FEAT, HMC_REG_FEAT__NUMBER_OF_BANKS_PER_VAULT, (unsigned)log2(num_banks) - 3);
  this->set_decoding(this->hmcsim_util_get_bsize(), num_banks); // initialize decoding!
}

hmc_register::~hmc_register(void)
{
}

bool hmc_register::hmcsim_get_decode_field(hmc_regslots_e name, struct hmcsim_reg_decode_fields_t **field)
{
  if (this->hmcsim_decode_fields[ name ].name == name) {
    *field = &this->hmcsim_decode_fields[ name ];
    return true;
  }
  else {
    for (unsigned i = 0; i < (unsigned)elemsof(this->hmcsim_decode_fields); i++) {
      if (this->hmcsim_decode_fields[ i ].name == name) {
        *field = &this->hmcsim_decode_fields[ i ];
        return true;
      }
    }
    return false;
  }
}

int hmc_register::hmcsim_get_decode_idx(unsigned reg)
{
  for (unsigned i = 0; i < elemsof(hmcsim_decode); i++) {
    if (reg == hmcsim_decode[i].reg)
      return hmcsim_decode[i].idx;
  }
  std::cerr << "ERROR: reg not defined " << reg << std::endl;
  return -1;
}

int hmc_register::hmcsim_reg_value_set_internal(unsigned reg_addr, hmc_regslots_e slot,
                                                uint64_t value, bool force_write)
{
  struct hmcsim_reg_decode_fields_t *field;
  if (!this->hmcsim_get_decode_field(slot, &field))
    return -1;

  if (!force_write && field->type == HMC_RO)
    return 0;

  int idx = hmcsim_get_decode_idx(reg_addr);
  if (idx == -1)
    return -1;

  uint32_t *reg = &this->regs[ idx ];
  uint32_t mask = (0x1 << field->size) - 1;
  *reg &= ~(mask << field->start_bit); // mask old value out
  *reg |= ((value & mask) << field->start_bit);

  if (slot == HMC_REG_AC__ADDRESS_MAPPING_MODE) {
    int bsize = this->hmcsim_util_decode_bsize(value);
    int banks = this->hmcsim_util_get_num_banks_per_vault();
    if (bsize == -1 || banks == -1)
      return -1;

    this->cube->set_decoding(bsize, banks);   // psiegl: speedup!

    /* in theory also writes to Cube Size, Number of Vaults and Number of Banks per Vault.
       But: these are RO!
     */
  }

  return 0;
}


int hmc_register::hmcsim_reg_value_set_full(unsigned reg_addr, uint64_t value)
{
  int idx = hmcsim_get_decode_idx(reg_addr);
  if (idx == -1)
    return -1;

  this->regs[ idx ] = value;

  if (reg_addr == HMC_REG_AC) {
    struct hmcsim_reg_decode_fields_t *field;
    if (!this->hmcsim_get_decode_field(HMC_REG_AC__ADDRESS_MAPPING_MODE, &field))
      return -1;

    int bsize = this->hmcsim_util_decode_bsize((value >> field->start_bit) & ((0x1 << field->size) - 1));
    int banks = this->hmcsim_util_get_num_banks_per_vault();
    if (banks == -1 || bsize == -1)
      return -1;

    this->cube->set_decoding(bsize, banks);   // psiegl: speedup!

    /* in theory also writes to Register FEATURES.
       But: all of the values are RO!
     */
  }

  return 0;
}

int hmc_register::hmcsim_reg_value_get(unsigned reg_addr, hmc_regslots_e slot, uint64_t *ret)
{
  struct hmcsim_reg_decode_fields_t *field;
  if (!this->hmcsim_get_decode_field(slot, &field))
    return -1;

  int idx = this->hmcsim_get_decode_idx(reg_addr);
  if (idx == -1)
    return -1;

  uint32_t *reg = &this->regs[ idx ];
  uint32_t mask = (0x1 << field->size) - 1;
  *ret = (*reg >> field->start_bit) & mask;

  return 0;
}

int hmc_register::hmcsim_reg_value_get_full(unsigned reg_addr, uint64_t *value)
{
  int idx = hmcsim_get_decode_idx(reg_addr);
  if (idx == -1)
    return -1;

  *value = this->regs[ idx ];

  return 0;
}

int hmc_register::hmcsim_util_decode_bsize(unsigned value)
{
  switch (value) {
  case 0x0:
  case 0x8:
    /* 32 bytes */
    return 32;
  case 0x1:
  case 0x9:
    /* 64 bytes */
    return 64;
  case 0x2:
  case 0xA:
    /* 128 bytes */
    return 128;
  case 0x3:   // HMC spec v2.1 doesn't provide details about registers anymore,
  // while 1.0, 1.1 don't contain bsize 256
  case 0xB:
    /* 256 bytes */
    return 256;
  case 0x4:
  case 0x5:
  case 0x6:
  case 0x7:
  case 0xC:
  case 0xD:
  case 0xE:
  case 0xF:
    /*
     * vendor specific
     *
     */
    return 0;
  default:
    std::cerr << "ERROR: No supported BSIZE in register" << std::endl;
    return -1;
  }
}
