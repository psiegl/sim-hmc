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

/* -------------------------------------------- TRACE VALUES */
#define   HMC_TRACE_BANK        (1<<0)
#define   HMC_TRACE_QUEUE       (1<<1)
#define   HMC_TRACE_CMD         (1<<2)
#define   HMC_TRACE_STALL       (1<<3)
#define   HMC_TRACE_LATENCY     (1<<4)

/* -------------------------------------------- MACROS */
#define   HMC_MAX_DEVS          8
#define   HMC_MAX_LINKS         4
#define   HMC_MIN_LINKS         2
#define   HMC_MAX_CAPACITY      8
#define   HMC_MIN_CAPACITY      4

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
#define   HMC_MAX_QUEUE_DEPTH   65536
#define   HMC_MAX_FLITS_PER_PACKET 17
#define   HMC_MAX_UQ_PACKET     (HMC_MAX_FLITS_PER_PACKET*2)

#define   HMC_MAX_CMDS          (1<<7)
#define   HMC_MAX_CMC           70

#define   HMC_1GB               (1ull<<30)


#define   HMC_HAS_LOGIC         1 /* inofficial */


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

#endif /* #ifndef _CONFIG_H_ */
