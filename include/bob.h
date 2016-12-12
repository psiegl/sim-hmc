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

#include <vector>
#include "bob_port.h"

using namespace std;

namespace BOBSim
{
class Transaction;
class BOBWrapper;
class DRAMChannel;
class BusPacket;

class bob_linkbus {
public:
  bob_linkbus(void) :
    serDesBuffer(0),
    inFlightLink(0),
    inFlightLinkCountdowns(0),
    linkIdle(0)
  {}
  ~bob_linkbus(void){}

  //SerDes buffer for holding incoming request packet
  Transaction* serDesBuffer;
  //The packet which is currently being sent
  Transaction* inFlightLink;
  //Coutner to determine how long packet is to be sent
  unsigned inFlightLinkCountdowns;
  //Counts cycles that link bus is idle
  unsigned linkIdle;
};

class BOB
{
private:
    //All DRAM channel objects (which includes ranks of DRAM and the simple contorller)
    DRAMChannel* channels[NUM_CHANNELS];
    unsigned num_ranks;

    //Bookkeeping for the number of requests to each channel
#ifndef BOBSIM_NO_LOG
    unsigned channelCounters[NUM_CHANNELS];
    uint64_t channelCountersLifetime[NUM_CHANNELS];
#endif

    //Storage for pending read request information
    vector<Transaction*> pendingReads;
#ifndef BOBSIM_NO_LOG
    unsigned pendingReadsBufferAvg;

    //Bookkeeping for port statistics
    vector<unsigned> portInputBufferAvg;
    vector<unsigned> portOutputBufferAvg;
#endif

    //
    //Request Link Bus
    //
    vector<bob_linkbus> reqLinkBus;

    //
    //Response Link Bus
    //
    vector<bob_linkbus> respLinkBus;

    //Round-robin counter
    unsigned responseLinkRoundRobin[NUM_LINK_BUSES];

    //Used for round-robin
    unsigned priorityPort;
    vector<unsigned> priorityLinkBus;

    //Bookkeeping
#ifndef BOBSIM_NO_LOG
    unsigned readCounter;
    unsigned writeCounter;
    unsigned committedWrites;
//	unsigned logicOpCounter;
#endif

    //Address mapping widths
#ifndef HMCSIM_SUPPORT
    unsigned rankBitWidth;
    unsigned bankBitWidth;
    unsigned rowBitWidth;
    unsigned colBitWidth;
    unsigned busOffsetBitWidth;
    unsigned channelBitWidth;
    unsigned cacheOffset;
#endif

    //Used to adjust for uneven clock frequencies
    unsigned clockCycleAdjustmentCounter;

    uint64_t dram_channel_clk;
    uint64_t currentClockCycle;
    unsigned num_ports;

    unsigned FindChannelID(Transaction* trans);
public:
	//Functions
    BOB(BOBWrapper *bobwrapper, unsigned num_ports, unsigned ranks, unsigned deviceWidth);
    ~BOB(void);
    void Update(void);
#ifndef BOBSIM_NO_LOG
	void PrintStats(ofstream &statsOut, ofstream &powerOut, bool finalPrint, unsigned elapsedCycles);
#endif
    void ReportCallback(BusPacket *bp);

    //Ports used on main BOB controller to communicate with cache
    vector<Port> ports;

    //Callback
    BOBWrapper *bobwrapper;
};
}
#endif
