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

#ifndef SIMPLECONTROLLER_H
#define SIMPLECONTROLLER_H

//Simple Controller header

#include <deque>
#include "bob_globals.h"

using namespace std;

namespace BOBSim
{
class Transaction;
class BusPacket;
class DRAMChannel;
class BankState;
class SimpleController
{
private:
    //Functions
    bool IsIssuable(BusPacket *busPacket);
#ifndef HMCSIM_SUPPORT
    void AddressMapping(uint64_t physicalAddress, unsigned *rank, unsigned *bank, unsigned *row, unsigned *col);
#endif

    //Fields
    DRAMChannel *channel;
    unsigned ranks;
    unsigned deviceWidth;

#ifndef HMCSIM_SUPPORT
    unsigned rankBitWidth;
    unsigned bankBitWidth;
    unsigned rowBitWidth;
    unsigned colBitWidth;
    unsigned busOffsetBitWidth;
    unsigned channelBitWidth;
    unsigned cacheOffset;
#endif

    uint64_t currentClockCycle;

    //Bank states for all banks in this channel
    vector< vector<BankState> > bankStates;

    //Storage and counters to determine write bursts
    vector< pair<unsigned, BusPacket*> > writeBurst; /* Countdown & Queue */

    //Sliding window for each rank to determine tFAW adherence
    vector< vector<unsigned> > tFAWWindow;

    //Refresh counters
    vector<unsigned> refreshCounters;

    //More bookkeeping
#ifndef BOBSIM_NO_LOG
    unsigned readCounter;
    unsigned writeCounter;
#endif

    //Work queue for pending requests (DRAM specific commands go here)
    deque<BusPacket*> commandQueue;

public:
	//Functions
    SimpleController(DRAMChannel *parent, unsigned num_ranks, unsigned deviceWidth);
    ~SimpleController(void);
    void Update(void); // this is called each tCK
    void AddTransaction(Transaction *trans);

#ifndef BOBSIM_NO_LOG
    void _update(void); // this is called each clk

    //Fields
    //Statistics and bookkeeping
    unsigned commandQueueMax;
    unsigned commandQueueAverage;
    unsigned numIdleBanksAverage;
    unsigned numActBanksAverage;
    unsigned numPreBanksAverage;
    unsigned numRefBanksAverage;
#endif

    unsigned RRQFull;
    unsigned outstandingReads;
    int waitingACTS;

    //Power fields
#ifndef BOBSIM_NO_LOG
    vector<uint64_t> backgroundEnergy;
    vector<uint64_t> burstEnergy;
    vector<uint64_t> actpreEnergy;
    vector<uint64_t> refreshEnergy;
#endif
};
}

#endif
