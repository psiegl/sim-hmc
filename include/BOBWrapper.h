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
#include <math.h>
#include "BOB.h"
#include "Transaction.h"

using namespace std;


namespace BOBSim
{
class BOBWrapper
{
private:
  void UpdateLatencyStats(Transaction *trans);

public:
	//Functions
    BOBWrapper(void);
    void Update(void);
    bool AddTransaction(Transaction* trans, unsigned port);
#if 0
	bool AddTransaction(uint64_t addr, bool isWrite, int coreID, void *logicOp);
#endif
	void RegisterCallbacks(
        void (*readDone)(unsigned, uint64_t),
        void (*writeDone)(unsigned, uint64_t),
        void (*logicDone)(unsigned, uint64_t));
    void PrintStats(bool finalPrint);
#if 0
	int FindOpenPort(uint coreID);
	bool isPortAvailable(unsigned port);
#endif

	//Fields
	//BOB object
	BOB *bob;
	//Incoming transactions being sent to each port
	vector<Transaction*> inFlightRequest;
	//Counters for determining how long packet should be sent
	vector<unsigned> inFlightRequestCounter;
	vector<unsigned> inFlightRequestHeaderCounter;

	//Outgoing transactiong being sent to cache
	vector<Transaction*> inFlightResponse;
	//Counters for determining how long packet should be sent
    vector<unsigned> inFlightResponseCounter;

	//Bookkeeping on request stream
	uint64_t maxReadsPerCycle;
	uint64_t maxWritesPerCycle;
	uint64_t readsPerCycle;
	uint64_t writesPerCycle;

	//Bookkeeping and statistics
	vector<unsigned> requestPortEmptyCount;
	vector<unsigned> responsePortEmptyCount;
	vector<unsigned> requestCounterPerPort;
	vector<unsigned> readsPerPort;
	vector<unsigned> writesPerPort;
	vector<unsigned> returnsPerPort;

	//Callback functions
    void (*readDoneCallback)(unsigned, uint64_t);
    void (*writeDoneCallback)(unsigned, uint64_t);
    void (*logicDoneCallback)(unsigned, uint64_t);

	//More bookkeeping and statistics
	vector< vector<unsigned> > perChanFullLatencies;
	vector< vector<unsigned> > perChanReqPort;
	vector< vector<unsigned> > perChanRspPort;
	vector< vector<unsigned> > perChanReqLink;
	vector< vector<unsigned> > perChanRspLink;
	vector< vector<unsigned> > perChanAccess;
	vector< vector<unsigned> > perChanRRQ;
	vector< vector<unsigned> > perChanWorkQTimes;

    vector<unsigned> fullLatencies;
    vector<unsigned> dramLatencies;
    vector<unsigned> chanLatencies;

	unsigned issuedLogicOperations;
	unsigned issuedWrites;
	unsigned committedWrites;
	unsigned returnedReads;
	unsigned totalReturnedReads;
	unsigned totalIssuedWrites;
	unsigned totalTransactionsServiced;
	unsigned totalLogicResponses;

	//Output files
	ofstream statsOut;
	ofstream powerOut;

	//Callback
	void WriteIssuedCallback(unsigned port, uint64_t address);
#if 0
	//Round-robin counter
	uint portRoundRobin;
#endif
    uint64_t currentClockCycle;
};
}


#endif
