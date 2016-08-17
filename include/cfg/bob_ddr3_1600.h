//
//DRAM Timing
//

//DDR3-1600 Micron Part : MT41J256M4-125E
//Clock Rate : 800MHz
#define NUM_RANKS  4
#define NUM_BANKS  8
#define NUM_ROWS  16384
#define NUM_COLS  2048

#define DEVICE_WIDTH  4
#define BL  8 //only used in power calculation

#define Vdd  1.5f

//CLOCK PERIOD
#define tCK  1.25f //ns

//in clock ticks
//ACT to READ or WRITE
#define tRCD  11
//PRE command period
#define tRP  11
//ACT to ACT
#define tRC  39
//ACT to PRE
#define tRAS  28

//CAS latency
#define tCL  11
//CAS Write latency
#define tCWL  8

//ACT to ACT (different banks)
#define tRRD  5
//4 ACT Window
#define tFAW  24
//WRITE recovery
#define tWR  12
//WRITE to READ
#define tWTR  6
//READ to PRE
#define tRTP  6
//CAS to CAS
#define tCCD  4
//REF to ACT
#define tRFC  88
//CMD time
#define tCMDS  1
//Rank to rank switch
#define tRTRS  2 //clk

//IDD Values
#define IDD0  95
#define IDD1  115
#define IDD2P0  12
#define IDD2P1  45
#define IDD2Q  67
#define IDD2N  70
#define IDD2NT  95
#define IDD3P  45
#define IDD3N  67
#define IDD4R  250
#define IDD4W  250
#define IDD5B  260
#define IDD6  6
#define IDD6ET  9
#define IDD7  400
#define IDD8  0
