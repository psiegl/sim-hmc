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

//DRAM Channel source

#include "../include/bob.h"
#include "../include/bob_dramchannel.h"
#include "../include/bob_rank.h"
#include "../include/bob_transaction.h"
#ifdef HMCSIM_SUPPORT
#include "../include/bob_wrapper.h"
#endif

using namespace std;
using namespace BOBSim;

DRAMChannel::DRAMChannel(unsigned id, BOB *_bob, unsigned num_ranks, unsigned deviceWidth) :
  logicLayer(LogicLayerInterface(id, this)),
  bob(_bob),
  inFlightCommandPacket(NULL),
  inFlightCommandCountdown(0),
  inFlightDataPacket(NULL),
  inFlightDataCountdown(0),
  simpleController(this, num_ranks, deviceWidth),
  channelID(id),
  pendingLogicResponse(NULL),
  readReturnQueueMax(0),
  DRAMBusIdleCount(0)
{
  for (unsigned i = 0; i < num_ranks; i++) {
    ranks.push_back(new Rank(i, this));
  }
}

DRAMChannel::~DRAMChannel(void)
{
  for (std::vector<Rank*>::iterator it = this->ranks.begin(); it < this->ranks.end(); ++it) {
    delete *it;
  }
  for (deque<BusPacket*>::iterator it = readReturnQueue.begin(); it != readReturnQueue.end(); ++it) {
    delete *it;
  }
//    if(pendingLogicResponse)
//      delete pendingLogicResponse;
//    if(inFlightCommandPacket)
//      delete inFlightCommandPacket;
//    if(inFlightDataPacket)
//      delete inFlightDataPacket;
}

void DRAMChannel::Update(void)
{
  //update buses
  if (inFlightCommandCountdown && !--inFlightCommandCountdown) {
    ranks[inFlightCommandPacket->rank]->ReceiveFromBus(inFlightCommandPacket);
    inFlightCommandPacket = NULL;
  }

  if (!inFlightDataCountdown) {
    DRAMBusIdleCount++;
  }
  else if (!--inFlightDataCountdown) {
    switch (inFlightDataPacket->busPacketType) {
    case READ_DATA:
      if (DEBUG_CHANNEL) DEBUG("     == Data burst complete");

      //if the bus packet was from a request originating from a logic operation, send it back to logic layer
      if (inFlightDataPacket->fromLogicOp) {
        logicLayer.ReceiveLogicOperation(new Transaction(RETURN_DATA, inFlightDataPacket->address, 64));
      }
      //if it was a regular request, add to return queue
      else {
        readReturnQueue.push_back(inFlightDataPacket);

        simpleController.outstandingReads--;

        bob->ReportCallback(inFlightDataPacket);
        //inFlightDataPacket = nullptr;

        //keep track of total number of entries in return queue
        if (readReturnQueue.size() > readReturnQueueMax) {
          readReturnQueueMax = readReturnQueue.size();
        }
      }
      break;
    case WRITE_DATA:
#ifdef HMCSIM_SUPPORT
      if (bob->bobwrapper->callback)
        bob->bobwrapper->callback(bob->bobwrapper->vault, inFlightDataPacket->payload);
#endif
      //bob->ReportCallback(inFlightDataPacket); // potentially uncomment
      ranks[inFlightDataPacket->rank]->ReceiveFromBus(inFlightDataPacket);
      break;
    default:
      ERROR("Encountered unexpected bus packet type");
      abort();
    }
  }

  //updates
  logicLayer.Update();

  simpleController.Update();
  for (std::vector<Rank*>::iterator it = this->ranks.begin(); it < this->ranks.end(); ++it) {
    (*it)->Update();
  }
}

bool DRAMChannel::AddTransaction(Transaction *trans)
{
  if (DEBUG_CHANNEL) DEBUG("    In AddTransaction - got");

  switch (trans->transactionType) {
  case LOGIC_OPERATION:
    logicLayer.ReceiveLogicOperation(trans);
    break;
  case LOGIC_RESPONSE:
    if (pendingLogicResponse == NULL) {
      if (DEBUG_LOGIC) DEBUG("== Made it back to channel ");
      pendingLogicResponse = trans;
      break;
    }
    else
      return false;
  default:
    if (simpleController.waitingACTS < CHANNEL_WORK_Q_MAX) {
      simpleController.AddTransaction(trans);
      break;
    }
    else
      return false;
  }

  return true;
}

void DRAMChannel::ReceiveOnCmdBus(BusPacket *busPacket)
{
  if (inFlightCommandCountdown) {
    ERROR("== Error - Bus collision while trying to receive from controller");
    exit(0);
  }

  //Report the time we waited in the queue
  switch (busPacket->busPacketType) {
  case ACTIVATE:
  case WRITE_P:   //Report the WRITE is finally going
    bob->ReportCallback(busPacket);
    break;
  default:
    break;
  }

  inFlightCommandPacket = busPacket;
  inFlightCommandCountdown = tCMDS;
}

void DRAMChannel::ReceiveOnDataBus(BusPacket *busPacket, bool is_return)
{
  switch (busPacket->busPacketType) {
  default:
    ERROR("== Error - Trying to put non-data packet on data bus!");
    exit(0);
  case READ_DATA:
  case WRITE_DATA:
    break;
  }

  if (inFlightDataCountdown) {
    ERROR("== Error - Bus collision while trying to receive from a rank in channel " << channelID);
    ERROR("               (Time Left) : " << inFlightDataCountdown);
    exit(0);
  }

  inFlightDataPacket = busPacket;
  if (is_return)
    inFlightDataCountdown = busPacket->respBurstSize();
  else
    inFlightDataCountdown = busPacket->reqBurstSize();
}
