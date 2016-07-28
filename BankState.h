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

#ifndef BANKSTATE_H
#define BANKSTATE_H

//Bank State header

#include "BusPacket.h"

namespace BOBSim
{
enum CurrentBankState
{
	IDLE,
	ROW_ACTIVE,
	PRECHARGING,
	REFRESHING
};

class BankState
{
public:
	//Functions
    BankState(void) :
      currentBankState(IDLE),
      openRowAddress(0),
      nextActivate(0),
      nextRead(0),
      nextWrite(0),
//      nextStrobeMin(0),
//      nextStrobeMax(0),
//      nextRefresh(0),
      stateChangeCountdown(0)
    {}

    void UpdateStateChange(void)
    {
      if(this->stateChangeCountdown)
      {
        this->stateChangeCountdown--;

        if( ! this->stateChangeCountdown )
        {
          switch(lastCommand)
          {
          case REFRESH:
            this->currentBankState = IDLE;
            break;
          case WRITE_P:
          case READ_P:
            this->currentBankState = PRECHARGING;
            this->stateChangeCountdown = tRP;
            this->lastCommand = PRECHARGE;
            break;
          case PRECHARGE:
            this->currentBankState = IDLE;
            break;
          default:
            ERROR("== WTF STATE? : "<<this->lastCommand);
            exit(0);
          }
        }
      }
    }

    bool isBankOpen(void) {
      return (this->currentBankState == ROW_ACTIVE ||
              this->currentBankState == REFRESHING);
    }

    void refresh(uint64_t cycle) {
      this->currentBankState = REFRESHING;
      this->stateChangeCountdown = tRFC;
      this->nextActivate = cycle + tRFC;
      this->lastCommand = REFRESH;
    }

	//Fields
	CurrentBankState currentBankState;
	unsigned openRowAddress;
	uint64_t nextActivate;
	uint64_t nextRead;
	uint64_t nextWrite;
//	uint64_t nextStrobeMin;
//	uint64_t nextStrobeMax;
//	uint64_t nextRefresh;     // ToDo!

	BusPacketType lastCommand;
	unsigned stateChangeCountdown;
};
}

#endif
