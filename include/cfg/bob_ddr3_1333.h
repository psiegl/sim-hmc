//
//DRAM Timing
//

//DDR3-1333 Micron Part : MT41J1G4-15E
//Clock Rate : 666MHz
#define NUM_RANKS  4 // per bank!
#define NUM_BANKS  8
#define NUM_ROWS  65536
#define NUM_COLS  2048

#define DEVICE_WIDTH  4
#define BL      8 //only used in power calculation

#define Vdd     1.5f

//CLOCK PERIOD
#define tCK     1.5f //ns

//in clock ticks
//ACT to READ or WRITE
#define tRCD    9   // 13.5ns
//PRE command period
#define tRP     9
//ACT to ACT
#define tRC     33
//ACT to PRE
#define tRAS    24  // 36 ns

//CAS latency
#define tCL     9
//CAS Write latency
#define tCWL    7

//ACT to ACT (different banks)
#define tRRD    4
//4 ACT Window
#define tFAW    20
//WRITE recovery
#define tWR     10
//WRITE to READ
#define tWTR    5
//READ to PRE
#define tRTP    5
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
