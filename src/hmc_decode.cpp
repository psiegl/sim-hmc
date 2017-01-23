#include <cassert>
#include <cstdint>
#include <cmath>
#include "hmc_decode.h"
#include "config.h"


hmc_decode::hmc_decode(void) :
  vault_shift(0),
  vault_mask(0),
  quad_shift(0),
  quad_mask(0),
  bank_shift(0),
  bank_mask(0),
  dram_shift_lo(0),
  dram_mask_lo(0),
  dram_shift_hi(0),
  dram_mask_hi(0),
  row_shift(0),
  row_mask(0),
  col_shift(0),
  col_mask(0)
{
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


   4GB:  32 BlockSize: dram [31:13], bank [12:10], quad [9:8],   vault [7:5],  dram[4]
         64 BlockSize: dram [31:14], bank [13:11], quad [10:9],  vault [8:6],  dram[5:4]
        128 BlockSize: dram [31:15], bank [14:12], quad [11:10], vault [9:7],  dram[6:4]
        256 BlockSize: dram [31:16], bank [15:13], quad [12:11], vault [10:8], dram[7:4]

   8GB:  32 BlockSize: dram [32:14], bank [13:10], quad [9:8],   vault [7:5],  dram[4]
         64 BlockSize: dram [32:15], bank [14:11], quad [10:9],  vault [8:6],  dram[5:4]
        128 BlockSize: dram [32:16], bank [15:12], quad [11:10], vault [9:7],  dram[6:4]
        256 BlockSize: dram [32:17], bank [16:13], quad [12:11], vault [10:8], dram[7:4]
 */
void hmc_decode::set_decoding(unsigned bsize, unsigned num_banks_per_vault)
{
  unsigned bit_start = (unsigned)log2(bsize);
  unsigned vault_mask_width = (unsigned)log2(HMC_NUM_VAULTS / HMC_NUM_QUADS);
  unsigned quad_mask_width = (unsigned)log2(HMC_NUM_QUADS);
  unsigned bank_mask_width = (unsigned)log2(num_banks_per_vault);
  unsigned begin_bit_start = bit_start;

  this->vault_shift = bit_start;
  this->vault_mask = (1 << vault_mask_width) - 1;

  bit_start += vault_mask_width;

  this->quad_shift = bit_start;
  this->quad_mask = (1 << quad_mask_width) - 1;

  bit_start += quad_mask_width;

  this->bank_shift = bit_start;
  this->bank_mask = (1 << bank_mask_width) - 1;

  bit_start += bank_mask_width;

  this->dram_shift_lo = 4;
  unsigned dram_mask_width_lo = begin_bit_start - this->dram_shift_lo;
  this->dram_mask_lo = (1 << dram_mask_width_lo) - 1;

  this->dram_shift_hi = bit_start - (dram_mask_width_lo - 1);
  unsigned dram_mask_width_hi = 20 - dram_mask_width_lo;
  this->dram_mask_hi = ((1 << dram_mask_width_hi) - 1) << dram_mask_width_lo;

// alignment [4:0] already removed from dram_mask, col and row need the same (according to bobsim only column!)
  unsigned col_mask_width = (unsigned)log2(HMC_NUM_COLS_PER_BANK) - this->dram_shift_lo;
  unsigned row_mask_width = (unsigned)log2(HMC_NUM_ROWS_PER_BANK);
  assert((dram_mask_width_lo + dram_mask_width_hi) == (col_mask_width + row_mask_width));

  // we first shuffle with DRAM, therewith we receive on junk of addr ... and then with ROW and COL
  // ToDo: col above, then row, or interleaved ..
  this->row_shift = 0;
  this->row_mask = (1 << row_mask_width) - 1;

  this->col_shift = row_mask_width;
  this->col_mask = (1 << col_mask_width) - 1;
}

