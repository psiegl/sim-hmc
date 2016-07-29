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

#include "LogicLayerInterface.h"

using namespace BOBSim;
using namespace std;

LogicLayerInterface::LogicLayerInterface(uint id, DRAMChannel *_channel):
    channel(_channel),
    simpleControllerID(id),
	currentClockCycle(0),
	currentTransaction(NULL),
    currentLogicOperation(NULL)
{}


void LogicLayerInterface::ReceiveLogicOperation(Transaction *trans)
{
    if (DEBUG_LOGIC) DEBUG("== Received in logic layer "<<simpleControllerID<<" on cycle "<<currentClockCycle);
	if (trans->transactionType == LOGIC_OPERATION)
	{
		pendingLogicOpsQueue.push_back(trans);
	}
	else
	{
		newOperationQueue.push_back(trans);
	}
}

void LogicLayerInterface::Update(void)
{
	//send back to channel if there is something in the outgoing queue
    if(outgoingQueue.size()>0 && channel->AddTransaction(*outgoingQueue.begin()))
	{
		//if we just send the response, we are done and can clear the current stuff
        if((*outgoingQueue.begin())->transactionType==LOGIC_RESPONSE)
		{
			currentTransaction = NULL;
			currentLogicOperation = NULL;
		}

		outgoingQueue.erase(outgoingQueue.begin());
	}

	// if we aren't working on anything and there are logic ops waiting
	if (currentTransaction == NULL && !pendingLogicOpsQueue.empty())
	{
		//cast logic operation arguments from transaction
        if((*pendingLogicOpsQueue.begin())->logicOpContents != NULL)
		{

          if(DEBUG_LOGIC) DEBUG(" == In logic layer "<<simpleControllerID<<" : interpreting transaction");

			//extract logic operation from transaction and grab handle to current operations
            currentTransaction = *pendingLogicOpsQueue.begin();
            currentLogicOperation = (LogicOperation *)currentTransaction->logicOpContents;
            currentLogicOperation->request(currentTransaction, &outgoingQueue);
		}
		else
		{
            ERROR(" == Error - Logic operation has no contents");
			exit(-1);
		}

        // grabbed the pointers, now we can remove from queue
        pendingLogicOpsQueue.pop_front();
	}


	//if we're getting data, it is probably from a logic op that generated requests
    if(!newOperationQueue.empty() && (*newOperationQueue.begin())->transactionType == RETURN_DATA)
	{
		//
		//do some checks to ensure the data makes sense
		//
		if(currentTransaction==NULL)
		{
			ERROR("== ERROR - Getting data without a corresponding transaction");
			exit(-1);
		}

        if (DEBUG_LOGIC) DEBUG("   ==[L:"<<simpleControllerID<<"] oh hai, return data");
		//handle the return data based on the logic op that is being executed

        currentLogicOperation->response(currentTransaction, &newOperationQueue, &outgoingQueue);
	}
	currentClockCycle++;
}
