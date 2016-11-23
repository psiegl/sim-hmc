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

//Rank source

#include <cstring>
#include "../include/bob_rank.h"
#include "../include/bob_dramchannel.h"

using namespace std;
using namespace BOBSim;

Rank::Rank(unsigned rankid, DRAMChannel *_channel):
    id(rankid),
    dramchannel(_channel),
    banksNeedUpdate(0),
    bankStates(new BankState[NUM_BANKS]),
    currentClockCycle(0)
{
    memset(bankStates, 0, sizeof(BankState) * NUM_BANKS);
}

Rank::~Rank(void)
{
    delete[] bankStates;
    for(vector< pair<unsigned, BusPacket*> >::iterator it = this->readReturn.begin(); it != this->readReturn.end(); ++it)
    {
       delete (*it).second;
    }
}

void Rank::Update(void)
{
    for(unsigned i=0; i<NUM_BANKS; i++)
    {
        bankStates[i].UpdateStateChange();
    }

    if(readReturn.size())
    {
        for(unsigned i=0; i<readReturn.size(); i++)
        {
            readReturn[i].first--;
        }
        if((*readReturn.begin()).first==0)
        {
            dramchannel->ReceiveOnDataBus((*readReturn.begin()).second);
            readReturn.erase(readReturn.begin());
        }
    }

	//increment clock cycle
	currentClockCycle++;
}

void Rank::ReceiveFromBus(BusPacket *busPacket)
{
	switch(busPacket->busPacketType)
	{
	case REFRESH:
		for(unsigned i=0; i<NUM_BANKS; i++)
		{
			if(bankStates[i].currentBankState != IDLE ||
               bankStates[i].nextActivate > currentClockCycle)
			{
				ERROR("== Error - Refresh when not allowed in bank "<<i);
				ERROR("           NextAct : "<<bankStates[i].nextActivate);
				ERROR("           State : "<<bankStates[i].currentBankState);
				exit(0);
			}

            bankStates[i].lastCommand = REFRESH;
			bankStates[i].currentBankState = REFRESHING;
            bankStates[i].stateChangeCountdown = tRFC;
			bankStates[i].nextActivate = currentClockCycle + tRFC;
		}
		delete busPacket;
		break;
    case ACTIVATE:
        if(bankStates[busPacket->bank].currentBankState != IDLE ||
                currentClockCycle < bankStates[busPacket->bank].nextActivate)
        {
            ERROR("== Error - Rank receiving ACT when not allowed");
            exit(0);
        }

        //
        //update bank states
        //
        bankStates[busPacket->bank].currentBankState = ROW_ACTIVE;
        bankStates[busPacket->bank].openRowAddress = busPacket->row;
        bankStates[busPacket->bank].nextRead = currentClockCycle + tRCD;
        bankStates[busPacket->bank].nextWrite = currentClockCycle + tRCD;
        bankStates[busPacket->bank].nextActivate = currentClockCycle + tRC;

        for(unsigned i=0; i<NUM_BANKS; i++)
        {
            if(i!=busPacket->bank)
            {
                bankStates[i].nextActivate = max(bankStates[i].nextActivate, currentClockCycle + tRRD);
            }
        }

        delete busPacket;
        break;
    case READ_P:
		if(bankStates[busPacket->bank].currentBankState != ROW_ACTIVE ||
           bankStates[busPacket->bank].openRowAddress != busPacket->row ||
           currentClockCycle < bankStates[busPacket->bank].nextRead)
		{
			ERROR("== Error - Rank receiving READ_P when not allowed");
			ERROR("           Current Clock Cycle : "<<currentClockCycle);
			exit(0);
		}

		//
		//update bankstates
		//
        busPacket->busPacketType = READ_DATA;
        readReturn.push_back( make_pair(tCL, busPacket) );

		for(unsigned i=0; i<NUM_BANKS; i++)
		{
			bankStates[i].nextRead = max(bankStates[i].nextRead, currentClockCycle + tCCD);
			bankStates[i].nextWrite = max(bankStates[i].nextWrite, currentClockCycle + tCCD);
		}

        bankStates[busPacket->bank].lastCommand = READ_P;
        bankStates[busPacket->bank].stateChangeCountdown = tRTP;
		bankStates[busPacket->bank].nextActivate = currentClockCycle + tRTP + tRP;
		bankStates[busPacket->bank].nextRead = bankStates[busPacket->bank].nextActivate;
		bankStates[busPacket->bank].nextWrite = bankStates[busPacket->bank].nextActivate;
		break;
	case WRITE_P:
		if(bankStates[busPacket->bank].currentBankState != ROW_ACTIVE ||
		        bankStates[busPacket->bank].openRowAddress != busPacket->row ||
		        currentClockCycle < bankStates[busPacket->bank].nextWrite)
		{
			ERROR("== Error - Rank "<<id<<" receiving WRITE_P when not allowed");
			ERROR("           currentClockCycle : "<<currentClockCycle);
			exit(0);
        }

		//
		//update bank states
		//
		for(unsigned i=0; i<NUM_BANKS; i++)
		{
			bankStates[i].nextRead = max(bankStates[i].nextRead, currentClockCycle + tCCD);
			bankStates[i].nextWrite = max(bankStates[i].nextWrite, currentClockCycle + tCCD);
		}
        bankStates[busPacket->bank].lastCommand = WRITE_P;
        bankStates[busPacket->bank].stateChangeCountdown = tCWL + busPacket->burstLength + tWR; // busPacket->burstLength == TRANSACTION_SIZE/DRAM_BUS_WIDTH
		bankStates[busPacket->bank].nextActivate = currentClockCycle + tCWL + busPacket->burstLength + tWR + tRP;
		bankStates[busPacket->bank].nextRead = bankStates[busPacket->bank].nextActivate;
		bankStates[busPacket->bank].nextWrite = bankStates[busPacket->bank].nextActivate;

		delete busPacket;
        break;
	case WRITE_DATA:
		if(bankStates[busPacket->bank].currentBankState != ROW_ACTIVE ||
           bankStates[busPacket->bank].openRowAddress != busPacket->row)
		{
			ERROR("== Error - Clock Cycle : "<<currentClockCycle);
            ERROR("== Error - Rank receiving WRITE_DATA when not allowed");
			exit(0);
        }

        bankStates[busPacket->bank].stateChangeCountdown = tWR;
		bankStates[busPacket->bank].nextActivate = currentClockCycle + tWR + tRP;

		delete busPacket;
		break;
	default:
		ERROR("== Error - Rank receiving incorrect type of bus packet");
		break;
	}
}
