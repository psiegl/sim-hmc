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

#ifndef BOBWRAPPER_H
#define BOBWRAPPER_H

#include <deque>
#include <cmath>
#include "bob.h"

using namespace std;


namespace BOBSim
{
class Transaction;

class clInFlightRequest {
public:
  clInFlightRequest(void) :
    Cache(0),
    Counter(0),
    HeaderCounter(0)
  {}
  //Incoming transactions being sent to each port
  Transaction* Cache;
  //Counters for determining how long packet should be sent
  unsigned Counter;
  unsigned HeaderCounter;
};

class clInFlightResponse {
public:
  clInFlightResponse(void) :
    Cache(0),
    Counter(0)
  {}
  //Outgoing transactiong being sent to cache
  Transaction* Cache;
  //Counters for determining how long packet should be sent
  unsigned Counter;
};

class BOBWrapper
{
private:
    //Fields
    //BOB object
    BOB bob;

    vector<clInFlightRequest> inFlightRequest;
    vector<clInFlightResponse> inFlightResponse;

    unsigned returnedReads;
    uint64_t returnedReadSize;
    unsigned totalReturnedReads;

    unsigned issuedWrites;
    uint64_t issuedWritesSize;
    unsigned totalIssuedWrites;

    //Bookkeeping and statistics
    vector<unsigned> requestPortEmptyCount;
    vector<unsigned> responsePortEmptyCount;
    vector<unsigned> requestCounterPerPort;
    vector<unsigned> readsPerPort;
    vector<unsigned> writesPerPort;
    vector<unsigned> returnsPerPort;

    //More bookkeeping and statistics
    class perChan {
    public:
      vector<unsigned> FullLatencies;
      double ReqPort;
      double RspPort;
      double ReqLink;
      double RspLink;
      double Access;
      double RRQ;
      double WorkQTimes;
    } perChan [NUM_CHANNELS];

    vector<unsigned> fullLatencies;
    vector<unsigned> dramLatencies;
    vector<unsigned> chanLatencies;

    unsigned issuedLogicOperations;
    unsigned committedWrites;
    unsigned totalTransactionsServiced;
    unsigned totalLogicResponses;

    //Output files
    ofstream statsOut;
    ofstream powerOut;

    uint64_t currentClockCycle;

    unsigned num_ports;

    void UpdateLatencyStats(Transaction *trans);

#if 0
    //Round-robin counter
    uint portRoundRobin;

    int FindOpenPort(uint coreID);
    bool isPortAvailable(unsigned port);
#endif

public:
#ifdef HMCSIM_SUPPORT
    BOBWrapper(unsigned num_ports, unsigned num_ranks, unsigned deviceWidth);
#else
    BOBWrapper(unsigned num_ports, unsigned num_ranks = NUM_RANKS, unsigned deviceWidth = DEVICE_WIDTH);
#endif
    ~BOBWrapper(void);
    void Update(void);
    bool AddTransaction(Transaction* trans, unsigned port);
#if 0
	bool AddTransaction(uint64_t addr, bool isWrite, int coreID, void *logicOp);
	void RegisterCallbacks(
        void (*readDone)(unsigned, uint64_t),
        void (*writeDone)(unsigned, uint64_t),
        void (*logicDone)(unsigned, uint64_t));
#endif
    void PrintStats(bool finalPrint);

    bool activatedPeriodPrintStates;

    //Callback functions
#if 0
    void (*readDoneCallback)(unsigned, uint64_t);
    void (*writeDoneCallback)(unsigned, uint64_t);
    void (*logicDoneCallback)(unsigned, uint64_t);
#endif

	//Callback
    void WriteIssuedCallback(unsigned port, uint64_t address);

#ifdef HMCSIM_SUPPORT
    bool IsPortBusy(unsigned port);
    bool (*callback)(void *vault, void *packet);
    void *vault;
#endif
};
}


#endif
