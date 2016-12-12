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

//Simple Controller source

#include <cmath>
#include <cstring>
#include "../include/bob_simplecontroller.h"
#include "../include/bob_dramchannel.h"
#include "../include/bob_buspacket.h"
#include "../include/bob_bankstate.h"
#include "../include/bob_buspacket.h"
#include "../include/bob_transaction.h"

using namespace std;
using namespace BOBSim;

SimpleController::SimpleController(DRAMChannel *parent, unsigned ranks, unsigned deviceWidth) :
  channel(parent),   //Registers the parent channel object
  ranks(ranks),
  deviceWidth(deviceWidth),

#ifndef HMCSIM_SUPPORT
  rankBitWidth(log2(ranks)),
  bankBitWidth(log2(NUM_BANKS)),
  rowBitWidth(log2(NUM_ROWS)),
  colBitWidth(log2(NUM_COLS)),
  busOffsetBitWidth(log2(BUS_ALIGNMENT_SIZE)),
  channelBitWidth(log2(NUM_CHANNELS)),
  cacheOffset(log2(CACHE_LINE_SIZE)),
#endif

  currentClockCycle(0),

  bankStates(ranks, vector<BankState>(NUM_BANKS, BankState())),
  tFAWWindow(ranks, vector<unsigned>(0)),

#ifndef BOBSIM_NO_LOG
  readCounter(0),
  writeCounter(0),
#endif
  //init power fields
#ifndef BOBSIM_NO_LOG_ENERGY
  backgroundEnergyOpenCtr(ranks, 0),
  backgroundEnergyCloseCtr(ranks, 0),
  burstEnergyCtr(ranks, 0),
  actpreEnergyCtr(ranks, 0),
  refreshEnergyCtr(ranks, 0),
#endif
#ifndef BOBSIM_NO_LOG
  commandQueueMax(0),
  commandQueueAverage(0),
  numIdleBanksAverage(0),
  numActBanksAverage(0),
  numPreBanksAverage(0),
  numRefBanksAverage(0),
  RRQFull(0),
#endif
  outstandingReads(0),

  waitingACTS(0)
{
  //Make the bank state objects
  for (unsigned i = 0; i < ranks; i++) {
    //init refresh counters
    refreshCounters.push_back(((7800 / tCK) / ranks) * (i + 1));
  }
}

SimpleController::~SimpleController(void)
{
  for (deque<BusPacket*>::iterator it = commandQueue.begin(); it != commandQueue.end(); ++it) {
    delete *it;
  }
  for (vector< pair<unsigned, BusPacket*> >::iterator it = writeBurst.begin(); it != writeBurst.end(); ++it) {
    delete (*it).second;
  }
}

#ifndef BOBSIM_NO_LOG
void SimpleController::_update(void)
{
  for (unsigned j = 0; j < this->commandQueue.size(); j++) {
    this->commandQueue[j]->queueWaitTime += (this->commandQueue[j]->busPacketType == ACTIVATE);
  }
}
#endif

//Updates the state of everything
void SimpleController::Update(void)
{
  //
  //Stats
  //
#ifndef BOBSIM_NO_LOG
  unsigned currentCount = 0;
  //count all the ACTIVATES waiting in the queue
  for (unsigned i = 0; i < commandQueue.size(); i++) {
    currentCount += (commandQueue[i]->busPacketType == ACTIVATE);
  }
  if (currentCount > commandQueueMax) commandQueueMax = currentCount;

  //cumulative rolling average
  commandQueueAverage += currentCount;
#endif

  for (unsigned r = 0; r < this->ranks; r++) {
#if !defined(BOBSIM_NO_LOG) || !defined(BOBSIM_NO_LOG_ENERGY)
    bool bankOpen = false;
#endif
    for (unsigned b = 0; b < NUM_BANKS; b++) {
#if !defined(BOBSIM_NO_LOG) || !defined(BOBSIM_NO_LOG_ENERGY)
      switch (bankStates[r][b].currentBankState) {
      //count the number of idle banks
      case IDLE:
#ifndef BOBSIM_NO_LOG
        numIdleBanksAverage++;
#endif
        break;

      //count the number of active banks
      case ROW_ACTIVE:
#ifndef BOBSIM_NO_LOG
        numActBanksAverage++;
#endif
#ifndef BOBSIM_NO_LOG_ENERGY
        bankOpen = true;
#endif
        break;

      //count the number of precharging banks
      case PRECHARGING:
#ifndef BOBSIM_NO_LOG
        numPreBanksAverage++;
#endif
        break;

      //count the number of refreshing banks
      case REFRESHING:
#ifndef BOBSIM_NO_LOG
        numRefBanksAverage++;
#endif
#ifndef BOBSIM_NO_LOG_ENERGY
        bankOpen = true;
#endif
        break;
      }
#endif

      //Updates the bank states for each rank
      bankStates[r][b].UpdateStateChange();
    }

    //
    //Power
    //
    //DRAM_BUS_WIDTH/2 because value accounts for DDR
#ifndef BOBSIM_NO_LOG_ENERGY
    if (bankOpen)
      backgroundEnergyOpenCtr[r]++;
    else
      backgroundEnergyCloseCtr[r]++;
#endif

    //
    //Update
    //
    //Updates the sliding window for tFAW
    for (unsigned i = 0; i < tFAWWindow[r].size(); i++) {
      if (!--tFAWWindow[r][i])
        tFAWWindow[r].erase(tFAWWindow[r].begin());
    }

    //Handle refresh counters
    refreshCounters[r] -= (refreshCounters[r] > 0);
  }

//Send write data to data bus
  for (unsigned i = 0; i < writeBurst.size(); i++) {
    writeBurst[i].first--;
  }
  if (writeBurst.size() && (*writeBurst.begin()).first == 0) {
    if (DEBUG_CHANNEL) DEBUG("     == Sending Write Data : ");
    channel->ReceiveOnDataBus((*writeBurst.begin()).second, false);
    writeBurst.erase(writeBurst.begin());
  }

  bool issuingRefresh = false;

//Figure out if everyone who needs a refresh can actually receive one
  for (unsigned r = 0; r < this->ranks; r++) {
    if (!refreshCounters[r]) {
      if (DEBUG_CHANNEL) DEBUG("      !! -- Rank " << r << " needs refresh");
      //Check to be sure we can issue a refresh
      bool canIssueRefresh = true;
      for (unsigned b = 0; b < NUM_BANKS; b++) {
        if (bankStates[r][b].nextActivate > currentClockCycle ||
            bankStates[r][b].currentBankState != IDLE) {
          canIssueRefresh = false;
          break;
        }
      }

      //Once all counters have reached 0 and everyone is either idle or ready to accept refresh-CAS
      if (canIssueRefresh) {
        if (DEBUG_CHANNEL) DEBUGN("-- !! Refresh is issuable - Sending : ");

        //BusPacketType packtype, unsigned transactionID, unsigned col, unsigned rw, unsigned r, unsigned b, unsigned prt, unsigned bl
        BusPacket *refreshPacket = new BusPacket(REFRESH, -1, 0, 0, r, 0, 0, channel->channelID, 0, false, 0);

        //Send to command bus
        channel->ReceiveOnCmdBus(refreshPacket);

        //make sure we don't send anythign else
        issuingRefresh = true;

#ifndef BOBSIM_NO_LOG_ENERGY
        refreshEnergyCtr[r]++;
#endif

        for (unsigned b = 0; b < NUM_BANKS; b++) {
          bankStates[r][b].currentBankState = REFRESHING;
          bankStates[r][b].stateChangeCountdown = tRFC;
          bankStates[r][b].nextActivate = currentClockCycle + tRFC;
          bankStates[r][b].lastCommand = REFRESH;
        }

        //reset refresh counters
        for (unsigned i = 0; i < this->ranks; i++) {
          if (!refreshCounters[r]) {
            refreshCounters[r] = 7800 / tCK;
          }
        }

        //only issue one
        break;
      }
    }
  }

//If no refresh is being issued then do this block
  if (!issuingRefresh) {
    for (unsigned i = 0; i < commandQueue.size(); i++) {
      //make sure we don't send a command ahead of its own ACTIVATE
      if ((!(i > 0 &&
             commandQueue[i]->transactionID == commandQueue[i - 1]->transactionID)) &&
          IsIssuable(commandQueue[i])) {     //Checks to see if this particular request can be issued
        //send to channel
        this->channel->ReceiveOnCmdBus(commandQueue[i]);

        //update channel controllers bank state bookkeeping
        unsigned rank = commandQueue[i]->rank;
        unsigned bank = commandQueue[i]->bank;

        //
        //Main block for determining what to do with each type of command
        //
        BankState *bankstate = &bankStates[rank][bank];
        unsigned burstLength = commandQueue[i]->reqBurstSize();
        switch (commandQueue[i]->busPacketType) {
        case READ_P:
          outstandingReads++;
          waitingACTS--;
          if (waitingACTS < 0) {
            ERROR("#@)($J@)#(RJ");
            exit(0);
          }

          //keep track of energy
#ifndef BOBSIM_NO_LOG_ENERGY
          burstEnergyCtr[rank]++;
#endif

          bankstate->lastCommand = commandQueue[i]->busPacketType;
          bankstate->stateChangeCountdown = (4 * tCK > 7.5) ? tRTP : ceil(7.5 / tCK); //4 clk or 7.5ns
          bankstate->nextActivate = max(bankstate->nextActivate, currentClockCycle + tRTP + tRP);
//					bankstate->nextRefresh = currentClockCycle + tRTP + tRP;

          for (unsigned r = 0; r < this->ranks; r++) {
            uint64_t read_offset;
            if (r == rank)
              read_offset = max((uint)tCCD, burstLength);
            else
              read_offset = burstLength + tRTRS;

            for (unsigned b = 0; b < NUM_BANKS; b++) {
              bankStates[r][b].nextRead = max(bankStates[r][b].nextRead,
                                              currentClockCycle + read_offset);
              bankStates[r][b].nextWrite = max(bankStates[r][b].nextWrite,
                                               currentClockCycle + (tCL + burstLength + tRTRS - tCWL));
            }
          }

          //prevents read or write being issued while waiting for auto-precharge to close page
          bankstate->nextRead = bankstate->nextActivate;
          bankstate->nextWrite = bankstate->nextActivate;
          break;
        case WRITE_P:
        {
          waitingACTS--;
          if (waitingACTS < 0) {
            ERROR(")(JWE)(FJEWF");
            exit(0);
          }

          //keep track of energy
#ifndef BOBSIM_NO_LOG_ENERGY
          burstEnergyCtr[rank] += (IDD4W - IDD3N) * BL / 2 * ((DRAM_BUS_WIDTH / 2 * 8) / this->deviceWidth);
#endif

          BusPacket *writeData = new BusPacket(*commandQueue[i]);
          writeData->busPacketType = WRITE_DATA;
          writeBurst.push_back(make_pair(tCWL, writeData));
          if (DEBUG_CHANNEL) DEBUG("     !!! After Issuing WRITE_P, burstQueue is :" << writeBurst.size() << " " << writeBurst.size() << " with head : " << (*writeBurst.begin()).second);

          bankstate->lastCommand = commandQueue[i]->busPacketType;
          unsigned stateChangeCountdown = tCWL + burstLength + tWR;
          bankstate->stateChangeCountdown = stateChangeCountdown;
          bankstate->nextActivate = currentClockCycle + stateChangeCountdown + tRP;
//			bankstate->nextRefresh = bankstate->nextActivate;

          for (unsigned r = 0; r < this->ranks; r++) {
            if (r == rank) {
              for (unsigned b = 0; b < NUM_BANKS; b++) {
                bankStates[r][b].nextRead = max(bankStates[r][b].nextRead,
                                                currentClockCycle + burstLength + tCWL + tWTR);
                bankStates[r][b].nextWrite = max(bankStates[r][b].nextWrite,
                                                 currentClockCycle + (uint64_t)max((uint)tCCD, burstLength));
              }
            }
            else {
              for (unsigned b = 0; b < NUM_BANKS; b++) {
                bankStates[r][b].nextRead = max(bankStates[r][b].nextRead,
                                                currentClockCycle + burstLength + tRTRS + tCWL - tCL);
                bankStates[r][b].nextWrite = max(bankStates[r][b].nextWrite,
                                                 currentClockCycle + burstLength + tRTRS);
              }
            }
          }

          //prevents read or write being issued while waiting for auto-precharge to close page
          bankstate->nextRead = bankstate->nextActivate;
          bankstate->nextWrite = bankstate->nextActivate;
          break;
        }
        case ACTIVATE:
          for (unsigned b = 0; b < NUM_BANKS; b++) {
            if (b != bank) {
              bankStates[rank][b].nextActivate = max(currentClockCycle + tRRD, bankStates[rank][b].nextActivate);
            }
          }

#ifndef BOBSIM_NO_LOG_ENERGY
          actpreEnergyCtr[rank]++;
#endif

          bankstate->lastCommand = commandQueue[i]->busPacketType;
          bankstate->currentBankState = ROW_ACTIVE;
          bankstate->openRowAddress = commandQueue[i]->row;
          bankstate->nextActivate = currentClockCycle + tRC;
          bankstate->nextRead = max(currentClockCycle + tRCD, bankstate->nextRead);
          bankstate->nextWrite = max(currentClockCycle + tRCD, bankstate->nextWrite);

          //keep track of sliding window
          tFAWWindow[rank].push_back(tFAW);

          break;
        default:
          ERROR("Unexpected packet type");
          abort();
        }

        //erase
        commandQueue.erase(commandQueue.begin() + i);

        break;
      }
    }
  }

//increment clock cycle
  currentClockCycle++;
}


bool SimpleController::IsIssuable(BusPacket *busPacket)
{
  unsigned rank = busPacket->rank;
  unsigned bank = busPacket->bank;
  BankState *bankState = &bankStates[rank][bank];

  //if((channel->readReturnQueue.size()+outstandingReads) * (busPacket->burstLength * DRAM_BUS_WIDTH) >= CHANNEL_RETURN_Q_MAX)
  //if((channel->readReturnQueue.size()) * (busPacket->burstLength * DRAM_BUS_WIDTH) >= CHANNEL_RETURN_Q_MAX)
  //	{
  //RRQFull++;
  //DEBUG("!!!!!!!!!"<<*busPacket)
  //exit(0);
  //return false;
  //}
  unsigned burstLength = busPacket->reqBurstSize();
  switch (busPacket->busPacketType) {
  case READ_P:
    if (bankState->currentBankState == ROW_ACTIVE &&
        bankState->openRowAddress == busPacket->row &&
        currentClockCycle >= bankState->nextRead &&
        (channel->readReturnQueue.size() + outstandingReads) * (burstLength * DRAM_BUS_WIDTH) < CHANNEL_RETURN_Q_MAX) {
      return true;
    }
    else {
#ifndef BOBSIM_NO_LOG
      RRQFull += ((channel->readReturnQueue.size() + outstandingReads) * (burstLength * DRAM_BUS_WIDTH) >= CHANNEL_RETURN_Q_MAX);
#endif
      return false;
    }

    break;
  case WRITE_P:
    if (bankState->currentBankState == ROW_ACTIVE &&
        bankState->openRowAddress == busPacket->row &&
        currentClockCycle >= bankState->nextWrite &&
        (channel->readReturnQueue.size() + outstandingReads) * (burstLength * DRAM_BUS_WIDTH) < CHANNEL_RETURN_Q_MAX) {
      return true;
    }
    else {
#ifndef BOBSIM_NO_LOG
      RRQFull += ((channel->readReturnQueue.size() + outstandingReads) * (burstLength * DRAM_BUS_WIDTH) >= CHANNEL_RETURN_Q_MAX);
#endif
      return false;
    }
    break;
  case ACTIVATE:
    return (bankState->currentBankState == IDLE &&
            currentClockCycle >= bankState->nextActivate &&
            refreshCounters[rank] > 0 &&
            tFAWWindow[rank].size() < 4);
  default:
    ERROR("== Error - Checking issuability on unknown packet type");
    exit(0);
  }
}

void SimpleController::AddTransaction(Transaction *trans)
{
  //map physical address to rank/bank/row/col
  unsigned mappedRank, mappedBank, mappedRow, mappedCol;
#ifdef HMCSIM_SUPPORT
  mappedRank = trans->rank;
  mappedBank = trans->bank;
  mappedRow = trans->row;
  mappedCol = trans->col;
#else
  AddressMapping(trans->address, &mappedRank, &mappedBank, &mappedRow, &mappedCol);
#endif

  bool originatedFromLogicOp = trans->originatedFromLogicOp;;
  BusPacket *action, *activate = new BusPacket(ACTIVATE, trans->transactionID, mappedCol, mappedRow, mappedRank, mappedBank, trans->portID, trans->mappedChannel, trans->address, trans->originatedFromLogicOp, 0);
  switch (trans->transactionType) {
  case DATA_READ:
#ifndef BOBSIM_NO_LOG
    readCounter++;
#endif
    action = new BusPacket(READ_P, trans->transactionID, mappedCol, mappedRow, mappedRank, mappedBank, trans->portID, trans->mappedChannel, trans->address, trans->originatedFromLogicOp, trans->reqSizeInBytes() / DRAM_BUS_WIDTH, trans->respSizeInBytes() / DRAM_BUS_WIDTH);

#ifdef HMCSIM_SUPPORT
    action->payload = trans->payload;
#endif
    break;
  case DATA_WRITE:
#ifndef BOBSIM_NO_LOG
    writeCounter++;
#endif
    action = new BusPacket(WRITE_P, trans->transactionID, mappedCol, mappedRow, mappedRank, mappedBank, trans->portID, trans->mappedChannel, trans->address, trans->originatedFromLogicOp, trans->reqSizeInBytes() / DRAM_BUS_WIDTH, trans->respSizeInBytes() / DRAM_BUS_WIDTH);

#ifdef HMCSIM_SUPPORT
    action->payload = trans->payload;
#endif
    delete trans;
    break;
  default:
    ERROR("== ERROR - Adding wrong transaction to simple controller");
    delete activate;
    exit(-1);
  }

  //if requests from logic ops have priority, put them at the front so they go first
  if (GIVE_LOGIC_PRIORITY && originatedFromLogicOp) {
    //create column write bus packet and add it to command queue
    this->commandQueue.push_front(action);
    //since we're pushing front, add the ACT after so it ends up being first
    this->commandQueue.push_front(activate);
  }
  else {
    //create the row activate bus packet and add it to command queue
    this->commandQueue.push_back(activate);
    //create column write bus packet and add it to command queue
    this->commandQueue.push_back(action);
  }

  waitingACTS++;
}

#ifndef HMCSIM_SUPPORT
void SimpleController::AddressMapping(uint64_t physicalAddress, unsigned *rank, unsigned *bank, unsigned *row, unsigned *col)
{
  uint64_t tempA, tempB;

  if (DEBUG_CHANNEL) DEBUGN("     == Mapped 0x" << hex << physicalAddress);

  switch (MAPPINGSCHEME) {
  case BK_CLH_RW_RK_CH_CLL_BY:  //bank:col_high:row:rank:chan:col_low:by
    //remove low order bits
    //this includes:
    // - byte offset
    // - low order bits of column address (assumed to be 0 since it is cache aligned)
    // - channel bits (already used)
    //
    //log2(CACHE_LINE_SIZE) == (log2(Low order column bits) + log2(BUS_ALIGNMENT_SIZE))
    physicalAddress >>= (channelBitWidth + cacheOffset);

    //rank bits
    tempA = physicalAddress;
    physicalAddress >>= rankBitWidth;
    tempB = physicalAddress << rankBitWidth;
    *rank = tempA ^ tempB;

    //row bits
    tempA = physicalAddress;
    physicalAddress >>= rowBitWidth;
    tempB = physicalAddress << rowBitWidth;
    *row = tempA ^ tempB;

    //column bits
    tempA = physicalAddress;
    physicalAddress >>= (colBitWidth - (cacheOffset - busOffsetBitWidth));
    tempB = physicalAddress << (colBitWidth - (cacheOffset - busOffsetBitWidth));
    *col = tempA ^ tempB;

    //account for low order column bits
    *col = *col << (cacheOffset - busOffsetBitWidth);

    //bank bits
    tempA = physicalAddress;
    physicalAddress >>= bankBitWidth;
    tempB = physicalAddress << bankBitWidth;
    *bank = tempA ^ tempB;

    break;
  case CLH_RW_RK_BK_CH_CLL_BY:  //col_high:row:rank:bank:chan:col_low:by
    //remove low order bits
    //this includes:
    // - byte offset
    // - low order bits of column address (assumed to be 0 since it is cache aligned)
    // - channel bits (already used)
    //
    //log2(CACHE_LINE_SIZE) == (log2(Low order column bits) + log2(BUS_ALIGNMENT_SIZE))
    physicalAddress >>= (channelBitWidth + cacheOffset);

    //bank bits
    tempA = physicalAddress;
    physicalAddress >>= bankBitWidth;
    tempB = physicalAddress << bankBitWidth;
    *bank = tempA ^ tempB;

    //rank bits
    tempA = physicalAddress;
    physicalAddress >>= rankBitWidth;
    tempB = physicalAddress << rankBitWidth;
    *rank = tempA ^ tempB;

    //row bits
    tempA = physicalAddress;
    physicalAddress >>= rowBitWidth;
    tempB = physicalAddress << rowBitWidth;
    *row = tempA ^ tempB;

    //column bits
    tempA = physicalAddress;
    physicalAddress >>= (colBitWidth - (cacheOffset - busOffsetBitWidth));
    tempB = physicalAddress << (colBitWidth - (cacheOffset - busOffsetBitWidth));
    *col = tempA ^ tempB;

    //account for low order column bits
    *col = *col << (cacheOffset - busOffsetBitWidth);

    break;
  case RK_BK_RW_CLH_CH_CLL_BY:  //rank:bank:row:colhigh:chan:collow:by
    //remove low order bits
    // - byte offset
    // - low order bits of column address (assumed to be 0 since it is cache aligned)
    // - channel bits (already used)
    //
    //log2(CACHE_LINE_SIZE) == (log2(Low order column bits) + log2(BUS_ALIGNMENT_SIZE))
    physicalAddress >>= (channelBitWidth + cacheOffset);

    //column bits
    tempA = physicalAddress;
    physicalAddress >>= (colBitWidth - (cacheOffset - busOffsetBitWidth));
    tempB = physicalAddress << (colBitWidth - (cacheOffset - busOffsetBitWidth));
    *col = tempA ^ tempB;

    //account for low order column bits
    *col = *col << (cacheOffset - busOffsetBitWidth);

    //row bits
    tempA = physicalAddress;
    physicalAddress >>= rowBitWidth;
    tempB = physicalAddress << rowBitWidth;
    *row = tempA ^ tempB;

    //bank bits
    tempA = physicalAddress;
    physicalAddress >>= bankBitWidth;
    tempB = physicalAddress << bankBitWidth;
    *bank = tempA ^ tempB;

    //rank bits
    tempA = physicalAddress;
    physicalAddress >>= rankBitWidth;
    tempB = physicalAddress << rankBitWidth;
    *rank = tempA ^ tempB;

    break;
  case RW_CH_BK_RK_CL_BY:  //row:chan:bank:rank:col:by
    //remove low order bits which account for the amount of data received on the bus (8 bytes)
    physicalAddress >>= busOffsetBitWidth;

    //column bits
    tempA = physicalAddress;
    physicalAddress >>= colBitWidth;
    tempB = physicalAddress << colBitWidth;
    *col = tempA ^ tempB;

    //rank bits
    tempA = physicalAddress;
    physicalAddress >>= rankBitWidth;
    tempB = physicalAddress << rankBitWidth;
    *rank = tempA ^ tempB;

    //bank bits
    tempA = physicalAddress;
    physicalAddress >>= bankBitWidth;
    tempB = physicalAddress << bankBitWidth;
    *bank = tempA ^ tempB;

    //channel has already been mapped so just shift off the bits
    physicalAddress >>= channelBitWidth;

    //row bits
    tempA = physicalAddress;
    physicalAddress >>= rowBitWidth;
    tempB = physicalAddress << rowBitWidth;
    *row = tempA ^ tempB;

    break;
  case RW_BK_RK_CH_CL_BY:  //row:bank:rank:chan:col:byte
    //remove low order bits which account for the amount of data received on the bus (8 bytes)
    physicalAddress >>= busOffsetBitWidth;

    //column bits
    tempA = physicalAddress;
    physicalAddress >>= colBitWidth;
    tempB = physicalAddress << colBitWidth;
    *col = tempA ^ tempB;

    //channel has already been mapped so just shift off the bits
    physicalAddress >>= channelBitWidth;

    //rank bits
    tempA = physicalAddress;
    physicalAddress >>= rankBitWidth;
    tempB = physicalAddress << rankBitWidth;
    *rank = tempA ^ tempB;

    //bank bits
    tempA = physicalAddress;
    physicalAddress >>= bankBitWidth;
    tempB = physicalAddress << bankBitWidth;
    *bank = tempA ^ tempB;

    //row bits
    tempA = physicalAddress;
    physicalAddress >>= rowBitWidth;
    tempB = physicalAddress << rowBitWidth;
    *row = tempA ^ tempB;

    break;
  case RW_BK_RK_CLH_CH_CLL_BY:  //row:bank:rank:col_high:chan:col_low:byte
    //remove low order bits
    //this includes:
    // - byte offset
    // - low order bits of column address (assumed to be 0 since it is cache aligned)
    // - channel bits (already used)
    //
    //log2(CACHE_LINE_SIZE) == (log2(Low order column bits) + log2(BUS_ALIGNMENT_SIZE))
    physicalAddress >>= (channelBitWidth + cacheOffset);

    //column bits
    tempA = physicalAddress;
    physicalAddress >>= (colBitWidth - (cacheOffset - busOffsetBitWidth));
    tempB = physicalAddress << (colBitWidth - (cacheOffset - busOffsetBitWidth));
    *col = tempA ^ tempB;

    //account for low order column bits
    *col = *col << (cacheOffset - busOffsetBitWidth);

    //rank bits
    tempA = physicalAddress;
    physicalAddress >>= rankBitWidth;
    tempB = physicalAddress << rankBitWidth;
    *rank = tempA ^ tempB;

    //bank bits
    tempA = physicalAddress;
    physicalAddress >>= bankBitWidth;
    tempB = physicalAddress << bankBitWidth;
    *bank = tempA ^ tempB;

    //row bits
    tempA = physicalAddress;
    physicalAddress >>= rowBitWidth;
    tempB = physicalAddress << rowBitWidth;
    *row = tempA ^ tempB;

    break;
  case RW_CLH_BK_RK_CH_CLL_BY:  //row:col_high:bank:rank:chan:col_low:byte
    //remove low order bits
    //this includes:
    // - byte offset
    // - low order bits of column address (assumed to be 0 since it is cache aligned)
    // - channel bits (already used)
    //
    //log2(CACHE_LINE_SIZE) == (log2(Low order column bits) + log2(BUS_ALIGNMENT_SIZE))
    physicalAddress >>= (channelBitWidth + cacheOffset);

    //rank bits
    tempA = physicalAddress;
    physicalAddress >>= rankBitWidth;
    tempB = physicalAddress << rankBitWidth;
    *rank = tempA ^ tempB;

    //bank bits
    tempA = physicalAddress;
    physicalAddress >>= bankBitWidth;
    tempB = physicalAddress << bankBitWidth;
    *bank = tempA ^ tempB;

    //column bits
    tempA = physicalAddress;
    physicalAddress >>= (colBitWidth - (cacheOffset - busOffsetBitWidth));
    tempB = physicalAddress << (colBitWidth - (cacheOffset - busOffsetBitWidth));
    *col = tempA ^ tempB;

    //account for low order column bits
    *col = *col << (cacheOffset - busOffsetBitWidth);

    //row bits
    tempA = physicalAddress;
    physicalAddress >>= rowBitWidth;
    tempB = physicalAddress << rowBitWidth;
    *row = tempA ^ tempB;

    break;
  case CH_RW_BK_RK_CL_BY:
    //remove bits which would address the amount of data received on a request
    physicalAddress >>= busOffsetBitWidth;

    //column bits
    tempA = physicalAddress;
    physicalAddress >>= colBitWidth;
    tempB = physicalAddress << colBitWidth;
    *col = tempA ^ tempB;

    //rank bits
    tempA = physicalAddress;
    physicalAddress >>= rankBitWidth;
    tempB = physicalAddress << rankBitWidth;
    *rank = tempA ^ tempB;

    //bank bits
    tempA = physicalAddress;
    physicalAddress >>= bankBitWidth;
    tempB = physicalAddress << bankBitWidth;
    *bank = tempA ^ tempB;

    //row bits
    tempA = physicalAddress;
    physicalAddress >>= rowBitWidth;
    tempB = physicalAddress << rowBitWidth;
    *row = tempA ^ tempB;

    break;
  default:
    ERROR("== ERROR - Unknown address mapping???");
    exit(1);
    break;
  }
  ;

  if (DEBUG_CHANNEL) DEBUG(" to RK:" << hex << *rank << " BK:" << *bank << " RW:" << *row << " CL:" << *col << dec);
}
#endif
