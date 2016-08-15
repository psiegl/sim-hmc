//
//DRAM Timing
//

//DDR3-1066 Micron Part : MT41J512M4-187E
//Clock Rate : 553MHz
#define NUM_RANKS   4
#define NUM_BANKS   8
#define NUM_ROWS   32768
#define NUM_COLS   2048

#define DEVICE_WIDTH   4
#define BL  8 //only used in power calculation

#define Vdd  1.5f

//CLOCK PERIOD
#define tCK  1.875f //ns

//in clock ticks
//ACT to READ or WRITE
#define tRCD  7 //13.125ns
//PRE command period
#define tRP  7 //13.125ns
//ACT to ACT
#define tRC  27 //50.625ns
//ACT to PRE
#define tRAS  20 //37.5ns

//CAS latency
#define tCL  7
//CAS Write latency
#define tCWL  6

//ACT to ACT (different banks)
#define tRRD  4 //7.5ns
//4 ACT Window
#define tFAW  20 //37.5ns
//WRITE recovery
#define tWR  8 //15ns
//WRITE to READ
#define tWTR  4 //7.5ns
//READ to PRE
#define tRTP  4 //7.5ns
//CAS to CAS
#define tCCD  4 //7.5ns
//REF to ACT
#define tRFC  86 //160ns
//CMD time
#define tCMDS  1 //clk
//Rank to rank switch
#define tRTRS  2 //clk

//IDD Values
#define IDD0  90
#define IDD1  115
#define IDD2P0  12
#define IDD2P1  35
#define IDD2Q  65
#define IDD2N  70
#define IDD2NT  90
#define IDD3P  55
#define IDD3N  80
#define IDD4R  200
#define IDD4W  255
#define IDD5B  290
#define IDD6  10
#define IDD6ET  14
#define IDD7  345
#define IDD8  0
