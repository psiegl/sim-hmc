#include <cstdint>
#include <math.h>
#include "hmc_decode.h"
#include "config.h"


hmc_decode::hmc_decode(void)
{
  // ToDo: issue default decoding
}

hmc_decode::~hmc_decode(void)
{

}

/*
    Byte address    Bytes within the maximum suppor-    The four LSBs of the byte address are ignored for READ and WRITE
                    ted block size                      requests (with the exception of BIT WRITE command; See BIT
                                                        WRITE command for details)

    Vault address   Addresses vaults within the HMC     Lower three bits of the vault address specifies 1 of 8 vaults within
                                                        the logic chip quadrant
                                                        Upper two bits of the vault address specifies 1 of 4 quadrants

    Bank address    Addresses banks within a vault      4GB HMC: Addresses 1 of 8 banks in the vault
                                                        8GB HMC: Addresses 1 of 16 banks in the vault

    DRAM address    Addresses DRAM rows and column      The vault controller breaks the DRAM address into row and col-
                    within a bank                       umn addresses, addressing 1Mb blocks of 16 bytes each


   4GB:   32 BlockSize: dram [31:13], bank [12:10], quad [9:8],   vault [7:5]
         64 BlockSize: dram [31:14], bank [13:11], quad [10:9],  vault [8:6]
        128 BlockSize: dram [31:15], bank [14:12], quad [11:10], vault [9:7]
        256 BlockSize: dram [31:16], bank [15:13], quad [12:11], vault [10:8]

   8GB:   32 BlockSize: dram [31:14], bank [13:10], quad [9:8],   vault [7:5]
         64 BlockSize: dram [31:15], bank [14:11], quad [10:9],  vault [8:6]
        128 BlockSize: dram [31:16], bank [15:12], quad [11:10], vault [9:7]
        256 BlockSize: dram [31:17], bank [16:13], quad [12:11], vault [10:8]
 */
void hmc_decode::set_decoding(unsigned bsize, unsigned num_banks_per_vault)
{
  unsigned bit_start = (unsigned)log2(bsize);
  unsigned vault_mask_width = (unsigned)log2(HMC_NUM_VAULTS / HMC_NUM_QUADS);
  unsigned quad_mask_width = (unsigned)log2(HMC_NUM_QUADS);
  unsigned bank_mask_width = (unsigned)log2(num_banks_per_vault);

  this->vault_shift = bit_start;
  this->vault_mask = (1 << vault_mask_width) - 1;

  bit_start += vault_mask_width;

  this->quad_shift = bit_start;
  this->quad_mask = (1 << quad_mask_width) - 1;

  bit_start += quad_mask_width;

  this->bank_shift = bit_start;
  this->bank_mask = (1 << bank_mask_width) - 1;

  bit_start += bank_mask_width;

  this->dram_shift = bit_start;
  unsigned dram_mask_width = 32 - this->dram_shift;
  this->dram_mask = (1 << dram_mask_width) - 1;
}

