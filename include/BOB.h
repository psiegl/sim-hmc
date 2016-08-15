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

#ifndef BOB_H
#define BOB_H

//BOB header

#include "Transaction.h"
#include "DRAMChannel.h"
#include "SimpleController.h"
#include "Port.h"

using namespace std;

namespace BOBSim
{
class BOBWrapper;
class BOB
{
public:
	//Functions
    BOB(BOBWrapper *_bobwrapper);
	unsigned FindChannelID(Transaction* trans);
    void Update(void);
	void PrintStats(ofstream &statsOut, ofstream &powerOut, bool finalPrint, unsigned elapsedCycles);
    void ReportCallback(BusPacket *bp);

	//Fields
	//Ports used on main BOB controller to communicate with cache
	vector<Port> ports;
	//All DRAM channel objects (which includes ranks of DRAM and the simple contorller)
    DRAMChannel* channels[NUM_CHANNELS];

	//Bookkeeping for the number of requests to each channel
    unsigned channelCounters[NUM_CHANNELS];
    uint64_t channelCountersLifetime[NUM_CHANNELS];

	//Storage for pending read request information
	vector<Transaction *> pendingReads;

	//Bookkeeping for port statistics
	vector<uint> portInputBufferAvg;
	vector<uint> portOutputBufferAvg;

	//
	//Request Link Bus
	//
	//SerDes buffer for holding outgoing request packet
    Transaction* serDesBufferRequest[NUM_LINK_BUSES];
	//The packet which is currently being sent
    Transaction* inFlightRequestLink[NUM_LINK_BUSES];
	//Counter to determine how long packet is to be sent
    unsigned inFlightRequestLinkCountdowns[NUM_LINK_BUSES];
	//Counts cycles that request link bus is idle
    unsigned requestLinkIdle[NUM_LINK_BUSES];

	//
	//Response Link Bus
	//
	//SerDes buffer for holding incoming request packet
    Transaction* serDesBufferResponse[NUM_LINK_BUSES];
	//The packet which is currently being sent
    Transaction* inFlightResponseLink[NUM_LINK_BUSES];
	//Coutner to determine how long packet is to be sent
    unsigned inFlightResponseLinkCountdowns[NUM_LINK_BUSES];
    //Counts cycles that response link bus is idle
    unsigned responseLinkIdle[NUM_LINK_BUSES];

	//Round-robin counter
    unsigned responseLinkRoundRobin[NUM_LINK_BUSES];

	//Used for round-robin 
	unsigned priorityPort;
	vector<unsigned> priorityLinkBus;

    //Bookkeeping
	unsigned readCounter;
	unsigned writeCounter;
	unsigned committedWrites;
	unsigned logicOpCounter;
	
    //Callback
    BOBWrapper *bobwrapper;

	//Address mapping widths 
	unsigned rankBitWidth;
	unsigned bankBitWidth;
	unsigned rowBitWidth;
	unsigned colBitWidth;
	unsigned busOffsetBitWidth;
	unsigned channelBitWidth;
	unsigned cacheOffset;

	//Used to adjust for uneven clock frequencies
	unsigned clockCycleAdjustmentCounter;

    uint64_t dram_channel_clk;
    uint64_t currentClockCycle;
};
}
#endif
