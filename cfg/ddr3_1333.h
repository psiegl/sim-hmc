//
//DRAM Timing
//

//DDR3-1333 Micron Part : MT41J1G4-15E
//Clock Rate : 666MHz
static uint NUM_RANKS = 4;
static uint NUM_BANKS = 8;
static ulong NUM_ROWS = 65536;
static ulong NUM_COLS = 2048;

static ulong DEVICE_WIDTH = 4;
static uint BL = 8; //only used in power calculation

static float Vdd = 1.5;

//CLOCK PERIOD
static float tCK = 1.5; //ns

//in clock ticks
//ACT to READ or WRITE
static uint tRCD = 9;
//PRE command period
static uint tRP = 9;
//ACT to ACT
static uint tRC = 33;
//ACT to PRE
static uint tRAS = 24;

//CAS latency
static uint tCL = 9;
//CAS Write latency
static uint tCWL = 7;

//ACT to ACT (different banks)
static uint tRRD = 4;
//4 ACT Window
static uint tFAW = 20;
//WRITE recovery
static uint tWR = 10;
//WRITE to READ
static uint tWTR = 5;
//READ to PRE
static uint tRTP = 5;
//CAS to CAS
static uint tCCD = 4;
//REF to ACT
static uint tRFC = 107;
//CMD time
static uint tCMDS = 1;
//Rank to rank switch
static uint tRTRS = 2; //clk

//IDD Values
static uint IDD0 = 75;
static uint IDD1 = 90;
static uint IDD2P0 = 12;
static uint IDD2P1 = 30;
static uint IDD2Q = 35;
static uint IDD2N = 40;
static uint IDD2NT = 55;
static uint IDD3P = 35;
static uint IDD3N = 45;
static uint IDD4R = 150;
static uint IDD4W = 155;
static uint IDD5B = 230;
static uint IDD6 = 12;
static uint IDD6ET = 16;
static uint IDD7 = 290;
static uint IDD8 = 0;
