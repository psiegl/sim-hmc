//
//DRAM Timing
//

//DDR3-1600 Micron Part : MT41J256M4-125E
//Clock Rate : 800MHz
static uint NUM_RANKS = 4;
static uint NUM_BANKS = 8;
static uint NUM_ROWS = 16384;
static uint NUM_COLS = 2048;

static uint DEVICE_WIDTH = 4;
static uint BL = 8; //only used in power calculation

static float Vdd = 1.5;

//CLOCK PERIOD
static float tCK = 1.25; //ns

//in clock ticks
//ACT to READ or WRITE
static uint tRCD = 11;
//PRE command period
static uint tRP = 11;
//ACT to ACT
static uint tRC = 39;
//ACT to PRE
static uint tRAS = 28;

//CAS latency
static uint tCL = 11;
//CAS Write latency
static uint tCWL = 8;

//ACT to ACT (different banks)
static uint tRRD = 5;
//4 ACT Window
static uint tFAW = 24;
//WRITE recovery
static uint tWR = 12;
//WRITE to READ
static uint tWTR = 6;
//READ to PRE
static uint tRTP = 6;
//CAS to CAS
static uint tCCD = 4;
//REF to ACT
static uint tRFC = 88;
//CMD time
static uint tCMDS = 1;
//Rank to rank switch
static uint tRTRS = 2; //clk

//IDD Values
static uint IDD0 = 95;
static uint IDD1 = 115;
static uint IDD2P0 = 12;
static uint IDD2P1 = 45;
static uint IDD2Q = 67;
static uint IDD2N = 70;
static uint IDD2NT = 95;
static uint IDD3P = 45;
static uint IDD3N = 67;
static uint IDD4R = 250;
static uint IDD4W = 250;
static uint IDD5B = 260;
static uint IDD6 = 6;
static uint IDD6ET = 9;
static uint IDD7 = 400;
static uint IDD8 = 0;
