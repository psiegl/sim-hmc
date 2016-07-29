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

#ifndef LOGICOPERATION_H
#define LOGICOPERATION_H

#include <vector>
#include <cstdlib>
#include <stdint.h>
#include <math.h>
#include "Globals.h"
#include "Transaction.h"

using namespace std;

namespace BOBSim
{
class LogicOperation
{
public:
	enum LogicType
	{
		PAGE_FILL,
		MEM_COPY,
		PAGE_TABLE_WALK
	};

	//functions
    LogicOperation(LogicType type, vector<uint64_t> &args) :
      logicType(type),
      arguments(args),
      issuedRequests(0)
    {}

    virtual bool request(Transaction *currentTransaction, vector<Transaction*> *outgoingQueue) = 0;
    virtual bool response(Transaction *currentTransaction, vector<Transaction *> *newOperationQueue, vector<Transaction*> *outgoingQueue) = 0;

	//fields
	LogicType logicType;
	vector<uint64_t> arguments;

    //keeps track of how many requests have been issued from this logic op
    uint issuedRequests;
};


class LogicOperation_PAGE_FILL : private LogicOperation
{
public:
  LogicOperation_PAGE_FILL(vector<uint64_t> &args) :
    LogicOperation(PAGE_FILL, args)
  {
    if(this->arguments.size()!=2)
    {
        ERROR("== ERROR - Incorrect number of arguments for logic operation "<<this->logicType);
        exit(-1);
    }
  }

  bool request(Transaction *currentTransaction, vector<Transaction*> *outgoingQueue)
  {
    //get arguments
    uint64_t pageStart = currentTransaction->address;

    //
    //Arguments:  1) Number of continuous pages
    //            2) Pattern to copy
    //
    uint64_t numPages = this->arguments[0];
//    uint pattern = (uint)this->arguments[1];

#if 0
    if(DEBUG_LOGIC)
    {
        DEBUG("       PAGE FILL");
        DEBUG("        Start     : "<<pageStart);
        DEBUG("        Num Pages : "<<numPages);

        DEBUG("        Fill with : "<<pattern);
    }
#endif

    //generate the writes required to write a pattern to each page
    for(int i=0; i<numPages; i++)
    {
        Transaction *trans = new Transaction(DATA_WRITE, 64, pageStart + i*(1<<((unsigned)(log2(BUS_ALIGNMENT_SIZE)+log2(NUM_CHANNELS)))));
        trans->originatedFromLogicOp = true;

//        if(DEBUG_LOGIC) DEBUG("      == Logic Op created");

        outgoingQueue->push_back(trans);

        //pattern not used lol :(
    }

    //put the response at the back of the queue so when all the writes empty out, the response is ready to go
    currentTransaction->transactionType = LOGIC_RESPONSE;
    outgoingQueue->push_back(currentTransaction);
    return true;
  }

  bool response(Transaction *currentTransaction, vector<Transaction *> *newOperationQueue, vector<Transaction*> *outgoingQueue)
  {
    return true;
  }
};


class LogicOperation_MEM_COPY : private LogicOperation
{
public:
  LogicOperation_MEM_COPY(vector<uint64_t> &args) :
    LogicOperation(MEM_COPY, args)
  {
    if(this->arguments.size()!=2)
    {
        ERROR("== ERROR - Incorrect number of arguments for logic operation "<<this->logicType);
        exit(-1);
    }
  }

  bool request(Transaction *currentTransaction, vector<Transaction*> *outgoingQueue)
  {
    //
    //Arguments : 1) Destination Address
    //            2) Size
    //
    uint64_t sourceAddress = currentTransaction->address;
//    uint64_t destinationAddress = this->arguments[0]; //start of destination copy
    uint64_t sizeToCopy = this->arguments[1];  //number of 64-byte words to copy

#if 0
    if(DEBUG_LOGIC)
    {
        DEBUG("       MEM COPY");
        DEBUG("        Source Address      : 0x"<<hex<<setw(8)<<setfill('0')<<sourceAddress);
        DEBUG("        Dest Address        : 0x"<<hex<<setw(8)<<setfill('0')<<destinationAddress);
        DEBUG("        Size to copy (x64B) : "<<dec<<sizeToCopy);
    }
#endif

    for(int i=0; i<sizeToCopy; i++)
    {
        Transaction *trans = new Transaction(DATA_READ, 64, sourceAddress + i*(1<<(log2(BUS_ALIGNMENT_SIZE)+log2(NUM_CHANNELS))));
        trans->originatedFromLogicOp = true;

//        if(DEBUG_LOGIC) DEBUG("      == Logic Op created");

        outgoingQueue->push_back(trans);
    }
    return true;
  }

  bool response(Transaction *currentTransaction, vector<Transaction *> *newOperationQueue, vector<Transaction*> *outgoingQueue)
  {
//    if (DEBUG_LOGIC) DEBUG((*newOperationQueue->begin())->address - currentTransaction->address);
    Transaction *t = new Transaction(DATA_WRITE, 64, (*newOperationQueue->begin())->address - currentTransaction->address + this->arguments[0]);
    t->originatedFromLogicOp = true;

//    if(DEBUG_LOGIC) DEBUG("      == Logic Op Copy moved data");

    outgoingQueue->push_back(t);

    issuedRequests++;

    newOperationQueue->erase(newOperationQueue->begin());

    //check to see if we are done with the requests
    if(issuedRequests==this->arguments[1])
    {
        if(DEBUG_LOGIC) DEBUG("      == All WRITE commands issued for copy - creating response");

        //send back response
        currentTransaction->transactionType = LOGIC_RESPONSE;
        outgoingQueue->push_back(currentTransaction);
        issuedRequests = 0;
    }
    return true;
  }
};


class LogicOperation_PAGE_TABLE_WALK : private LogicOperation
{
public:
  LogicOperation_PAGE_TABLE_WALK(vector<uint64_t> &args) :
    LogicOperation(PAGE_TABLE_WALK, args)
  {
  }

  bool request(Transaction *currentTransaction, vector<Transaction*> *outgoingQueue)
  {
//    uint64_t src_address = currentTransaction->address;
    for (size_t i=0; i<this->arguments.size(); i++)
    {
        Transaction *trans = new Transaction(DATA_READ, 64, this->arguments[i]);
        trans->originatedFromLogicOp = true;
        outgoingQueue->push_back(trans);
    }
    return true;
  }

  bool response(Transaction *currentTransaction, vector<Transaction *> *newOperationQueue, vector<Transaction*> *outgoingQueue)
  {
    issuedRequests++;

//    if (DEBUG_LOGIC) DEBUG("   ==[L:"<<simpleControllerID<<"] Getting back PT read for 0x"<<std::hex<<(newOperationQueue[0]->address)<<std::dec<<"("<<issuedRequests<<"/3)");

    //check to see if we are done with the requests
    if(issuedRequests == this->arguments.size())
    {
//        if(DEBUG_LOGIC) DEBUG("      ==[L:"<<simpleControllerID<<"] All walked all page table levels, - creating response");

        //send back response
        currentTransaction->transactionType = LOGIC_RESPONSE;
        outgoingQueue->push_back(currentTransaction);
        issuedRequests = 0;
    }
    newOperationQueue->erase(newOperationQueue->begin());
    return true;
  }
};


}

#endif
