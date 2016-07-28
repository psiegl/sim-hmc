//
//DRAM Timing
//

//DDR3-1066 Micron Part : MT41J512M4-187E
//Clock Rate : 553MHz
static uint NUM_RANKS = 4;
static uint NUM_BANKS = 8;
static uint NUM_ROWS = 32768;
static uint NUM_COLS = 2048;

static uint DEVICE_WIDTH = 4;
static uint BL = 8; //only used in power calculation

static float Vdd = 1.5;

//CLOCK PERIOD
static float tCK = 1.875; //ns

//in clock ticks
//ACT to READ or WRITE
static uint tRCD = 7; //13.125ns
//PRE command period
static uint tRP = 7; //13.125ns
//ACT to ACT
static uint tRC = 27; //50.625ns
//ACT to PRE
static uint tRAS = 20; //37.5ns

//CAS latency
static uint tCL = 7;
//CAS Write latency
static uint tCWL = 6;

//ACT to ACT (different banks)
static uint tRRD = 4; //7.5ns
//4 ACT Window
static uint tFAW = 20; //37.5ns
//WRITE recovery
static uint tWR = 8; //15ns
//WRITE to READ
static uint tWTR = 4; //7.5ns
//READ to PRE
static uint tRTP = 4; //7.5ns
//CAS to CAS
static uint tCCD = 4; //7.5ns
//REF to ACT
static uint tRFC = 86; //160ns
//CMD time
static uint tCMDS = 1; //clk
//Rank to rank switch
static uint tRTRS = 2; //clk

//IDD Values
static uint IDD0 = 90;
static uint IDD1 = 115;
static uint IDD2P0 = 12;
static uint IDD2P1 = 35;
static uint IDD2Q = 65;
static uint IDD2N = 70;
static uint IDD2NT = 90;
static uint IDD3P = 55;
static uint IDD3N = 80;
static uint IDD4R = 200;
static uint IDD4W = 255;
static uint IDD5B = 290;
static uint IDD6 = 10;
static uint IDD6ET = 14;
static uint IDD7 = 345;
static uint IDD8 = 0;
