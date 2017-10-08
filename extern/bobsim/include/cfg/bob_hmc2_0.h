//
//DRAM Timing
//

// ------------------------
// from HMC spec
// ------------------------
#define V_ss     0.0f // (V) Ground
#define V_dd     0.9f // (V) Logic Core Source
#define V_tt     1.2f // (V) Link TX termination source
#define V_tr     1.2f // (V) Link RX termination source
#define V_ddplla 1.2f // (V) Link PLLA source
#define V_ddpllb 1.2f // (V) Link PLLB source
#define V_ddpllr 1.2f // (V) Intermediate frequency PLL source
#define V_ddm    1.2f // (V) DRAM source
#define V_ccp    2.5f // (V) DRAM wordline boost source
#define V_ddk    1.5f // (V) JTAG, NVM, I2C source


/*
/DRAM timing model
#define tRRD_s                                                      4   //Activate to activate command delay (same strata)
//#define tRRD_d                                                      1   //Activate to activate command delay (diff strata)
//#define tFAW                                                      n/a
//#define tRFC_all                                                    200 //Refresh all banks to activate or refresh command delay
//#define tRFC_bank                                                   44  //refresh per bank to activate or refresh command to same bank delay
//#define tRFC_d                                                      4 //Refresh per bank to activate or refresh command to different bank delay
#define tRCDr                                                       17  //Activate to read command delay
#define tRCDw                                                       2   //Activate to write command delay
#define tAA                                                         14  //Open page read latency
//#define tRCDr+tAA                                                 31  //closed page read latency
#define tCWL                                                        12  // open page write latency
//#define tRCDw+tCWL                                                14  //closed page read latency
#define tRAS                                                        28  //activate to precharge command delay
#define tRP                                                         16  // precharge to activate command delay
#define tRC                                                         44  // Activate to activate or refresh command to same bank delay
//#define tRTP                                                        1   //read to precharge command delay
//#define tWR                                                         18  //delay from end of write strobe to precharge command
#define tW2P                                                        34  //write to precharge command delay
#define tCCD_s                                                      4   //Read to read command to same strata or write to write command delay
#define tCCD_d                                                      6   //Read to read command to different strata delay
//#define tWTR_s                                                      8   //end of write strobe to read command to same bank delay
//#define tWTR_d                                                    n/a  //end of write strobe to read command to diff bank delay
#define tW2R_s                                                      24  //Write to read command to same bank delay
#define tW2R_d                                                      3   //Write to read command to diff bank delay
#define tRTW                                                        9   //read to write command delay
//#define tAC                                                         1   //1UI clock to read data
 */

// ------------------------
// from CACTI 7.0 3DD
// ------------------------
#define NUM_RANKS     4 // == stacked die count 4 -> 8GB, 2 -> 4GB
#define NUM_BANKS     8 // == number of banks
#define NUM_ROWS      1024
#define NUM_COLS      512

#define DEVICE_WIDTH  4 // == chip IO width?
#define BL            8 // == bursth depth?

#define Vdd           V_ddm // since the bobsim will only represent the DRAM source

//CLOCK PERIOD
#define tCK           1.5f // ns 666 MHz

//in clock ticks
//ACT to READ or WRITE
#define tRCD          (unsigned)(5.49162/tCK)   // previous 9  // -- RAS to CAS delay
//PRE command period
#define tRP           (unsigned)(3.39136/tCK)   // previous 9  // -- RAS precharge
//ACT to ACT
#define tRC           (unsigned)(14.2552/tCK)   // previous 33
//ACT to PRE
#define tRAS          (unsigned)(11.6056/tCK)   // previous 24 // -- RAS active time

//CAS latency
#define tCL           (unsigned)(14.2552/tCK)   // previous 9  // -- CAS latency
//CAS Write latency
#define tCWL    7

//ACT to ACT (different banks)
#define tRRD          (unsigned)(4.35353/tCK)   // previous 4  // -- RAS to RAS delay
//4 ACT Window
#define tFAW    20
//WRITE recovery
#define tWR     10 // -- Write recovery time
//WRITE to READ
#define tWTR    5  // -- Write to read delay
//READ to PRE
#define tRTP    5  // -- Read to precharge delay
//CAS to CAS
#define tCCD    4
//REF to ACT
#define tRFC    107
//CMD time
#define tCMDS   1
//Rank to rank switch
#define tRTRS   2 //clk

// names of IDD: High-Level Power Estimation and Optimization of DRAMs, Karthik Chandrasekar
//          and: Viking Technology, DDR3 VLP UDIMM VR7VUxx7258xBx
//          and: Micron 4Gb: x4, x8, x16 DDR3 SDRAM
//IDD Values
#define IDD0    75    // mA One Bank Activate-Precharge Current
//#define IDD1    90  // mA One Bank Activate-Read-Precharge current
//#define IDD2P0  12  // mA Precharge power-down current: Slow exit
//#define IDD2P1  30  // mA Precharge power-down current: Fast exit
//#define IDD2Q   35  // mA Precharge quiet standby current;
#define IDD2N   40    // mA Precharge Standby Current
//#define IDD2NT  55  // mA Precharge standby ODT current
//#define IDD3P   35  // mA activate power-down standby current
#define IDD3N   45    // mA Activate Standby Current
#define IDD4R   150   // mA Burst Read Current
#define IDD4W   155   // mA Burst Write Current
#define IDD5B   230   // mA Burst Refresh Current
//#define IDD6    12  // mA Self Refresh Current
//#define IDD6ET  16  // mA Extended Temperature Range Self-Refresh Current;
//#define IDD7    290 // mA Operating bank interleave read current
//#define IDD8    0   // mA Deep Power Down Current
