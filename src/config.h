#ifndef _CONFIG_H_
#define _CONFIG_H_

/* -------------------------------------------- VERSION MACROS */
#ifndef   HMC_MAJOR_VERSION
#define   HMC_MAJOR_VERSION     2
#endif

#ifndef   HMC_MINOR_VERSION
#define   HMC_MINOR_VERSION     0
#endif

#ifndef   HMC_APPEND_VERSION
#define   HMC_APPEND_VERSION    ""
#endif

/* -------------------------------------------- VENDOR ID DATA */
#ifndef   HMC_VENDOR_ID
#define   HMC_VENDOR_ID         0xF
#endif

#ifndef   HMC_PRODUCT_REVISION
#define   HMC_PRODUCT_REVISION  0x2
#endif

#ifndef   HMC_PROTOCOL_REVISION
#define   HMC_PROTOCOL_REVISION 0x2
#endif

#ifndef   HMC_PHY_REVISION
#define   HMC_PHY_REVISION      0x1
#endif

/* -------------------------------------------- PHYSICAL MACRO DATA */
#ifndef   HMC_PHY_SPEED
#define   HMC_PHY_SPEED         0x0
#endif

/* -------------------------------------------- MACROS */
#define   HMC_MAX_DEVS          8
#define   HMC_MAX_LINKS         4
#define   HMC_MIN_LINKS         2
#define   HMC_MAX_CAPACITY      8
#define   HMC_MIN_CAPACITY      4
#define   HMC_CLK_PERIOD_NS   0.8f //1.25 GHz


/*
HMC2.1
page 35

Each link is associated with eight local vaults that are referred to as a quadrant. Access
from a link to a local quadrant may have lower latency than to a link outside the quad-
rant. Bits within the vault address designate both the specific quadrant and the vaults
within that quadrant.

Upper two bits of the vault address specifies 1 of 4 quadrants

=> 1 Quadrant: Mapping: 8 local vaults + 1 link + 3 remote Quadrants
*/

#define   HMC_NUM_VAULTS        32
#define   HMC_NUM_QUADS         4

/*
HMC2.1

page 38
Each bank in the HMC contains 16,777,216 memory bytes (16MB). A WRITE or READ
command to a bank accesses 32 bytes of data for each column fetch. The bank configu-
ration is identical in all specified HMC configurations.

page 35
The vault controller breaks the DRAM address into row and col-
umn addresses, addressing 1Mb blocks of 16 bytes each
*/

#define   HMC_MAX_BANKS         512 // BANKS in sum (needs to be divided per VAULTs!)  --> 8 GB HMC
#define   HMC_MIN_BANKS         256 // BANKS in sum (needs to be divided per VAULTs!)  --> 4 GB HMC

// The vault controller breaks the DRAM address into row and column addresses, addressing 1Mb blocks of 16 bytes each
// MT41J256M16: either 64K:2K, 64K:1K, 32K:1K
// MT41J256M4:  either 16K:2K, 16K:1K,  8K:1K
#define   HMC_NUM_ROWS_PER_BANK 16384 // not specified by HMC spec, but typical value
#define   HMC_NUM_COLS_PER_BANK 1024  // not specified by HMC spec, but typical value
#define   HMC_NUM_BANKS_PER_RANK 2

#define   HMC_MAX_QUEUE_DEPTH   65536
#define   HMC_MAX_FLITS_PER_PACKET 17
#define   HMC_MAX_UQ_PACKET     (HMC_MAX_FLITS_PER_PACKET*2)

#define   HMC_MAX_CMDS          (1<<7)
#define   HMC_MAX_CMC           70

//#define   HMC_HAS_LOGIC         1 /* inofficial */

#define   FLIT_WIDTH            128 /* bit */

#define   RETRY_BUFFER_FLITS    256 /* flits */

/* registers */
#define   HMC_NUM_REGS          26

#define   HMC_REG_EDR__BASE     0x2B0000
#define   HMC_REG_EDR__OFFSET   0x000001
#define   HMC_REG_EDR( i )      (HMC_REG_EDR__BASE + (( i ) * HMC_REG_EDR__OFFSET))

#define   HMC_REG_ERR           0x2B0004
#define   HMC_REG_GC            0x280000

#define   HMC_REG_LC__BASE      0x240000
#define   HMC_REG_LC__OFFSET    0x010000
#define   HMC_REG_LC( i )       (HMC_REG_LC__BASE + (( i ) * HMC_REG_LC__OFFSET))

#define   HMC_REG_LRLL__BASE    0x240003
#define   HMC_REG_LRLL__OFFSET  0x010000
#define   HMC_REG_LRLL( i )    (HMC_REG_LRLL__BASE + (( i ) * HMC_REG_LRLL__OFFSET))

#define   HMC_REG_LR__BASE      0x0C0000
#define   HMC_REG_LR__OFFSET    0x010000
#define   HMC_REG_LR( i )      (HMC_REG_LR__BASE + (( i ) * HMC_REG_LR__OFFSET))

#define   HMC_REG_IBTC__BASE    0x040000
#define   HMC_REG_IBTC__OFFSET  0x010000
#define   HMC_REG_IBTC( i )     (HMC_REG_IBTC__BASE + (( i ) * HMC_REG_IBTC__OFFSET))

#define   HMC_REG_AC            0x2C0000
#define   HMC_REG_VCR           0x108000
#define   HMC_REG_FEAT          0x2C0003
#define   HMC_REG_RVID          0x2C0004

/*
  Commands and data are transmitted in both directions across the link using a packet-
  based protocol where the packets consist of 128-bit flow units called “FLITs.” These
  FLITs are serialized, transmitted across the physical lanes of the link, then re-assembled
  at the receiving end of the link. Three conceptual layers handle packet transfers:
  • The physical layer handles serialization, transmission, and deserialization.
  • The link layer provides the low-level handling of the packets at each end of the link.
  • The transaction layer provides the definition of the packets, the fields within the
  packets, and the packet verification and retry functions of the link.
 */

/*
 * Thesis:
 *
 * 125 MHz (REF CLK)
 * 10 GB/s -> vault clock: 10GB/s / (32 TSV data lanes) = 2.5Gb/s data rate (assume: double data rate transmission)
 * yields: 1.25 GHz TSV Frequency t_CK = 0.8ns --> 8x 100ps -> 1 FLIT! (128bit for 1 clk)
 * frequency is precisely 10 times the link ref. clock of 125 MHz
 *
 * uses timings:
 *         value (cycles @    Time (ns)   value (cycles @
 *         t_CK = 1.25ns)                 t_CK = 0.8ns)
 * t_RP    11 cycles          13.75ns     17 cycles
 * t_CCD   4 cycles           5ns         6 cycles
 * t_RCD   11 cycles          13.75ns     17 cycles
 * t_CL    11 cycles          13.75ns     17 cycles
 * t_WR    12 cycles          15ns        19 cycles
 * t_RAS   22 cycles          27.5ns      34 cycles
 *
 *
 *
 *
 *

CACTI3DD
Timing Components:
       t_RCD (Row to column command delay): 5.49162 ns              ~ 4 cycles (1.25ns)
       t_RAS (Row access strobe latency): 11.6056 ns                ~ 9 cycles (1.25ns)
       t_RC (Row cycle): 14.2552 ns                                 ~ 11 cycles (1.25ns)
       t_CAS (Column access strobe latency): 26.9434 ns
       t_RP (Row precharge latency): 3.39136 ns                     ~ 3 cycles (1.25ns)
       t_RRD (Row activation to row activation delay): 4.35353 ns
Power Components:
       Activation energy: 2.30653 nJ
       Read energy: 1.52285 nJ
       Write energy: 1.52314 nJ
       Precharge energy: 1.03846 nJ

Time Components:

     row activation bus delay (ns): 3.61179
     row predecoder delay (ns): 0.689387
     row decoder delay (ns): 0.516858
     local wordline delay (ns): 0.287216
     bitline delay (ns): 3.62016
     sense amp delay (ns): 0.325648
     column access bus delay (ns): 3.61179
     column predecoder delay (ns): 0.357764
     column decoder delay (ns): 2.1111
     datapath bus delay (ns): 3.61179
     global dataline delay (ns): 1.79881
     local dataline delay (ns): 7.44249
     data buffer delay (ns): 2.95421
     subarray output driver delay (ns): 3.43528

Energy Components:

     row activation bus energy (nJ): 0.113767
     row predecoder energy (nJ): 0.000458332
     row decoder energy (nJ): 0.000286932
     local wordline energy (nJ): 0.000182402
     bitline energy (nJ): 0.943563
     sense amp energy (nJ): 1.20011
     column access bus energy (nJ): 0.0985983
     column predecoder energy (nJ): 4.63255e-05
     column decoder energy (nJ): 0.000387437
     column selectline energy (nJ): 0.0734667
     datapath bus energy (nJ): 0.242703
     global dataline energy (nJ): 0.0114835
     local dataline energy (nJ): 0.585444
     data buffer energy (nJ): 0.00346825

*/

#endif /* #ifndef _CONFIG_H_ */
