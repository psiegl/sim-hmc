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
#include "bob_transaction.h"

using namespace std;

namespace BOBSim
{
class BOB;
class BOBWrapper
{
private:
    //Fields
    //BOB object
    BOB *bob;

#if 0
    //Round-robin counter
    uint portRoundRobin;

    int FindOpenPort(uint coreID);
    bool isPortAvailable(unsigned port);
#endif
    unsigned returnedReads;
    uint64_t returnedReadSize;
    unsigned totalReturnedReads;

    unsigned issuedWrites;
    uint64_t issuedWritesSize;
    unsigned totalIssuedWrites;

    unsigned num_ports;
    void UpdateLatencyStats(Transaction *trans);

public:
	//Functions
    BOBWrapper(unsigned num_ports);
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

    struct __attribute__ ((packed)) {
      //Incoming transactions being sent to each port
      Transaction** Cache;
      //Counters for determining how long packet should be sent
      unsigned* Counter;
      unsigned* HeaderCounter;
    } inFlightRequest;

    struct __attribute__ ((packed)) {
      //Outgoing transactiong being sent to cache
      Transaction** Cache;
      //Counters for determining how long packet should be sent
      unsigned* Counter;
    } inFlightResponse;

	//Bookkeeping and statistics
    unsigned* requestPortEmptyCount;
    unsigned* responsePortEmptyCount;
    unsigned* requestCounterPerPort;
    unsigned* readsPerPort;
    unsigned* writesPerPort;
    unsigned* returnsPerPort;

	//Callback functions
#if 0
    void (*readDoneCallback)(unsigned, uint64_t);
    void (*writeDoneCallback)(unsigned, uint64_t);
    void (*logicDoneCallback)(unsigned, uint64_t);
#endif
private:
	//More bookkeeping and statistics
    struct  __attribute__ ((packed)) {
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
public:
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
