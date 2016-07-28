/*********************************************************************************
*  Copyright (c) 2010-2011, Elliott Cooper-Balis
*                             Paul Rosenfeld
*                             Bruce Jacob
*                             University of Maryland 
*                             dramninjas [at] gmail [dot] com
*  All rights reserved.
*  
*  Redistribution and use in source and binary forms, with or without
*  modification, are permitted provided that the following conditions are met:
*  
*     * Redistributions of source code must retain the above copyright notice,
*        this list of conditions and the following disclaimer.
*  
*     * Redistributions in binary form must reproduce the above copyright notice,
*        this list of conditions and the following disclaimer in the documentation
*        and/or other materials provided with the distribution.
*  
*  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
*  ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
*  WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
*  DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
*  FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
*  DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
*  SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
*  CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
*  OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
*  OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*********************************************************************************/

#ifndef GLOBALS_H
#define GLOBALS_H

//Global fields header

#include <iostream>
#include <iomanip>
#include <fstream>
#include <vector>
#include <string>
#include <cstdlib>
#include <stdint.h>

#ifdef LOG_OUTPUT
#define PRINT(str)  {logOutput<< str<<std::endl;}
#define PRINTN(str) {logOutput<< str;}
#define DEBUG(str)  {if (BOBSim::SHOW_SIM_OUTPUT) {logOutput<< str <<std::endl;}}
#define DEBUGN(str) {if (BOBSim::SHOW_SIM_OUTPUT) {logOutput<< str;}}
namespace BOBSim
{
extern std::ofstream logOutput; //defined in BOBWrapper.cpp
}
#else

#ifdef NO_OUTPUT
#define PRINT(str)
#define PRINTN(str)
#define DEBUG(str)
#define DEBUGN(str)

#else
#define PRINT(str)  { std::cout <<str<<std::endl; }
#define PRINTN(str) { std::cout <<str; }
#define DEBUG(str)  {if (BOBSim::SHOW_SIM_OUTPUT) {std::cout<< str <<std::endl;}}
#define DEBUGN(str) {if (BOBSim::SHOW_SIM_OUTPUT) {std::cout<< str;}}
#endif
#endif

#ifdef LOG_OUTPUT
#endif

#define ERROR(str) std::cerr<<"[ERROR ("<<__FILE__<<":"<<__LINE__<<")]: "<<str<<std::endl;

namespace BOBSim
{
enum PortHeuristicScheme
{
	FIRST_AVAILABLE,
	ROUND_ROBIN,
	PER_CORE
};

enum AddressMappingScheme
{
	RW_BK_RK_CH_CL_BY, //row:bank:rank:chan:col:byte
	RW_CH_BK_RK_CL_BY, //row:chan:bank:rank:col:byte
	RW_BK_RK_CLH_CH_CLL_BY, //row:bank:rank:col_high:chan:col_low:byte
	RW_CLH_BK_RK_CH_CLL_BY, //row:col_high:bank:rank:chan:col_low:byte
	CH_RW_BK_RK_CL_BY, //chan:row:bank:rank:col:byte
	RK_BK_RW_CLH_CH_CLL_BY, //chan:rank:bank:row:col:byte
	CLH_RW_RK_BK_CH_CLL_BY, //col_high:row:rank:bank:chan:col_low:byte
	BK_CLH_RW_RK_CH_CLL_BY //bank:col_high:row:rank:chan:col_low:byte
};

//
//Debug Flags
//
//Prints debug information relating to the DRAM channel
static bool DEBUG_CHANNEL = false;
//Prints debug information relating to the main BOB controller and link buses 
static bool DEBUG_BOB =  false;
//Prints debug information relating to the ports on the main BOB controller
static bool DEBUG_PORTS = false;
//prints debug information relating to the logic layer in each simple controller
static bool DEBUG_LOGIC = false;

//
//Random Stream Sim stuff
//
//Number of CPU cycles between epoch boundaries.  
static uint EPOCH_LENGTH = 1000000; 
//Flag for quiet mode (-q)
extern int SHOW_SIM_OUTPUT;

//
//BOB Architecture Config
//
//
//NOTE : NUM_LINK_BUSES * CHANNELS_PER_LINK_BUS = NUM_CHANNELS
//
//Number of link buses in the system
#define NUM_LINK_BUSES                4
//Number of DRAM channels in the system
#define NUM_CHANNELS                  8
//Multi-channel optimization degree
#define CHANNELS_PER_LINK_BUS (NUM_CHANNELS / NUM_LINK_BUSES)

//Number of lanes for both request and response link bus
#define REQUEST_LINK_BUS_WIDTH        8 //Bit Lanes
#define RESPONSE_LINK_BUS_WIDTH      12 //Bit Lanes

//Clock frequency for link buses
//static float LINK_BUS_CLK_PERIOD = .15625; // ns - 6.4 GHz
static float LINK_BUS_CLK_PERIOD = .3125; // ns - 3.2 GHz
//Ratio between CPU and link bus clocks - computed at runtime
extern uint LINK_CPU_CLK_RATIO;
//Flag to turn on/off double-data rate transfer on link bus
static bool LINK_BUS_USE_DDR = true;

//Size of DRAM request
#define TRANSACTION_SIZE             64 // psiegl: changes as of packet size!
//Width of DRAM bus as standardized by JEDEC
#define DRAM_BUS_WIDTH               16 //bytes - DOUBLE TO ACCOUNT FOR DDR - 64-bits wide JEDEC bus

//Number of ports on main BOB controller
extern uint NUM_PORTS;
//Number of bytes each port can transfer in a CPU cycle
#define PORT_WIDTH                   16
//Number of transaction packets that each port buffer may hold
#define PORT_QUEUE_DEPTH              8
//Heuristic used for adding new requests to the available ports
static PortHeuristicScheme portHeuristic = FIRST_AVAILABLE;

//Number of requests each simple controller can hold in its work queue
#define CHANNEL_WORK_Q_MAX           16 //entries
//Amount of response data that can be held in each simple controller return queue
#define CHANNEL_RETURN_Q_MAX       1024 //bytes

//
//Logic Layer Stuff
//
//Lets logic responses be sent before regular data requests
#define GIVE_LOGIC_PRIORITY        true
//Size of logic request packet
#define LOGIC_RESPONSE_PACKET_OVERHEAD 8

//
//Packet sizes
//
//Packet overhead for requests and responses
#define RD_REQUEST_PACKET_OVERHEAD   16  //bytes (orig: 8)
#define RD_RESPONSE_PACKET_OVERHEAD  16 //bytes (orig: 8)
#define WR_REQUEST_PACKET_OVERHEAD   16 //bytes (orig: 8)

//
//Power
//
//Average power consumption of a single simple controller package
#define SIMP_CONT_BACKGROUND_POWER    7 //watts
//Additional power consumption for each simple controller core in the package
#define SIMP_CONT_CORE_POWER        3.5 //watts

//
//DRAM Stuff
//
//Alignment to determine width of DRAM bus, used in mapping
#define BUS_ALIGNMENT_SIZE           8
//Cache line size in bytes 
#define CACHE_LINE_SIZE             64
//Offset of channel ID
#define CHANNEL_ID_OFFSET            0
//Ratio of clock speeds between DRAM and CPU - computed at runtime
extern uint DRAM_CPU_CLK_RATIO;
//Address mapping scheme - defined at the top of this file
#define MAPPINGSCHEME RW_CLH_BK_RK_CH_CLL_BY

//
//CPU
//
//CPU clock frequency in nanoseconds
#define CPU_CLK_PERIOD          0.3125 //ns

//
//DRAM Timing
//
#if defined(DDR3_1333)
#include "cfg/ddr3_1333.h"
#elif defined(DDR3_1600)
#include "cfg/ddr3_1600.h"
#elif defined(DDR3_1066)
#include "cfg/ddr3_1066.h"
#endif

uint inline log2(unsigned value)
{
	uint logbase2 = 0;
	unsigned orig = value;
	value>>=1;
	while(value>0)
	{
		value >>= 1;
		logbase2++;
	}
	if(1<<logbase2<orig)logbase2++;
	return logbase2;
}

uint inline log2_64(uint64_t value)
{
	uint logbase2 = 0;
	uint64_t orig = value;
	value>>=1;
	while(value>0)
	{
		value>>=1;
		logbase2++;
	}
	if(1<<logbase2<orig)logbase2++;
	return logbase2;
}
}

#endif
