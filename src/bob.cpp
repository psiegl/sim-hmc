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

//BOB source
#include <bitset>
#include <cstring>
#include <cmath>
#include "../include/bob.h"
#include "../include/bob_wrapper.h"
#include "../include/bob_simplecontroller.h"
#include "../include/bob_transaction.h"
#include "../include/bob_dramchannel.h"
#include "../include/bob_buspacket.h"

using namespace std;

namespace BOBSim
{
#ifdef LOG_OUTPUT
ofstream logOutput;
#endif

unsigned DRAM_CPU_CLK_RATIO;
unsigned DRAM_CPU_CLK_ADJUSTMENT;
BOB::BOB(BOBWrapper *bobwrapper, unsigned num_ports, unsigned ranks, unsigned deviceWidth) :
    num_ranks(ranks),
    portInputBufferAvg(num_ports, 0),
    portOutputBufferAvg(num_ports, 0),

    //Initialize fields for bookkeeping
    reqLinkBus(NUM_LINK_BUSES, bob_linkbus()),
    respLinkBus(NUM_LINK_BUSES, bob_linkbus()),

    priorityPort(0),
    priorityLinkBus(num_ports, 0), //Used for round-robin

    readCounter(0),
    writeCounter(0),
    committedWrites(0),
    clockCycleAdjustmentCounter(0),
#ifndef HMCSIM_SUPPORT
    rankBitWidth(log2(ranks)),
    bankBitWidth(log2(NUM_BANKS)),
    rowBitWidth(log2(NUM_ROWS)),
    colBitWidth(log2(NUM_COLS)),
    busOffsetBitWidth(log2(BUS_ALIGNMENT_SIZE)),
    channelBitWidth(log2(NUM_CHANNELS)),
    cacheOffset(log2(CACHE_LINE_SIZE)),
#endif

    num_ports(num_ports),
    ports(num_ports, Port()), //Make port objects
    bobwrapper(bobwrapper)
{
    dram_channel_clk = 0;
    currentClockCycle = 0;

#ifdef LOG_OUTPUT
    string output_filename("BOBsim");
    if (getenv("SIM_DESC"))
        output_filename += getenv("SIM_DESC");
    output_filename += ".log";
    logOutput.open(output_filename.c_str());
#endif
#ifndef HMCSIM_SUPPORT
    DEBUG("busoff:"<<busOffsetBitWidth<<" col:"<<colBitWidth<<" row:"<<rowBitWidth<<" rank:"<<rankBitWidth<<" bank:"<<bankBitWidth<<" chan:"<<channelBitWidth);
#endif

    //Compute ratios of various clock frequencies
    if(fmod(CPU_CLK_PERIOD,LINK_BUS_CLK_PERIOD)>0.00001)
    {
        ERROR("== ERROR - Link bus clock period ("<<LINK_BUS_CLK_PERIOD<<") must be a factor of CPU clock period ("<<CPU_CLK_PERIOD<<")");
        ERROR("==         fmod="<<fmod(CPU_CLK_PERIOD,LINK_BUS_CLK_PERIOD));
        abort();
    }

    if(LINK_CPU_CLK_RATIO!=1)
      cout<<"Channel to CPU ratio : "<<LINK_CPU_CLK_RATIO<<endl;
    DRAM_CPU_CLK_RATIO = tCK / CPU_CLK_PERIOD;
    if(DRAM_CPU_CLK_RATIO!=1)
      cout<<"DRAM to CPU ratio : "<<DRAM_CPU_CLK_RATIO<<endl;
    DRAM_CPU_CLK_ADJUSTMENT = tCK / fmod(tCK,CPU_CLK_PERIOD);
#ifndef HMCSIM_SUPPORT
    if(DRAM_CPU_CLK_ADJUSTMENT!=1)
      cout<<"DRAM_CPU_CLK_ADJUSTMENT : "<<DRAM_CPU_CLK_ADJUSTMENT<<endl;
#endif

    //Ensure that parameters have been set correclty
    if(NUM_CHANNELS / CHANNELS_PER_LINK_BUS != NUM_LINK_BUSES)
    {
        ERROR("== ERROR - Mismatch in NUM_CHANNELS and NUM_LINK_BUSES");
        exit(0);
    }


    memset(responseLinkRoundRobin, 0, sizeof(unsigned) * NUM_LINK_BUSES);

    memset(channelCounters, 0, sizeof(unsigned) * NUM_CHANNELS);
    memset(channelCountersLifetime, 0, sizeof(uint64_t) * NUM_CHANNELS);

    //Create channels
    for(unsigned i=0; i<NUM_CHANNELS; i++)
    {
        channels[i] = new DRAMChannel(i, this, num_ranks, deviceWidth);
    }
}

BOB::~BOB(void)
{
    for(unsigned i=0; i<NUM_CHANNELS; i++)
    {
        delete channels[i];
    }

    for(vector<Transaction*>::iterator it = pendingReads.begin(); it!=pendingReads.end(); ++it)
    {
      delete *it;
    }
}

void BOB::Update(void)
{
    //
    //STATS KEEPING
    //

    //keep track of how long data has been waiting in the channel for the switch to become available
    for(unsigned i=0; i<pendingReads.size(); i++)
    {
        //look in the channel that the request was mapped to and search its return queue
        unsigned mappedChannel = pendingReads[i]->mappedChannel;
        for(unsigned j=0; j<channels[mappedChannel]->readReturnQueue.size(); j++)
        {
            //if the transaction IDs match, then the data is waiting there to return (it might not be ready yet,
            //  which would not trigger this condition)
            pendingReads[i]->cyclesInReadReturnQ += (pendingReads[i]->transactionID==channels[pendingReads[i]->mappedChannel]->readReturnQueue[j]->transactionID);
        }
    }

    //calculate number of transactions waiting
    for(unsigned i=0; i<NUM_CHANNELS; i++)
    {
        for(unsigned j=0; j<channels[i]->simpleController.commandQueue.size(); j++)
        {
            channels[i]->simpleController.commandQueue[j]->queueWaitTime += (channels[i]->simpleController.commandQueue[j]->busPacketType==ACTIVATE);
        }
    }

    //keep track of idle link buses
    for(unsigned i=0; i<NUM_LINK_BUSES; i++)
    {
        reqLinkBus[i].linkIdle += (reqLinkBus[i].inFlightLink==NULL);
        respLinkBus[i].linkIdle += (respLinkBus[i].inFlightLink==NULL);
    }

    //keep track of average entries in port in/out-buffers
    for(unsigned i=0; i<this->num_ports; i++)
    {
        portInputBufferAvg[i] += ports[i].inputBuffer.size();
        portOutputBufferAvg[i] += ports[i].outputBuffer.size();

        //
        // UPDATE BUSES
        //
        //update each port's bookkeeping

        if(ports[i].inputBusyCountdown>0)
        {
            ports[i].inputBusyCountdown--;

            //if the port input is done sending, erase that item
//			if(ports[i].inputBusyCountdown==0)
//			{
//
//			}
        }

        ports[i].outputBusyCountdown -= (ports[i].outputBusyCountdown>0);
    }

    for(unsigned i=0; i<NUM_LINK_BUSES; i++)
    {
        bob_linkbus *i_reqLinkBus = &reqLinkBus[i];
        if(i_reqLinkBus->inFlightLinkCountdowns && !--i_reqLinkBus->inFlightLinkCountdowns)
        {
            //compute total time in serDes and travel up channel
            i_reqLinkBus->inFlightLink->cyclesReqLink = currentClockCycle - i_reqLinkBus->inFlightLink->cyclesReqLink;

            //add to channel
            channels[i_reqLinkBus->inFlightLink->mappedChannel]->AddTransaction(i_reqLinkBus->inFlightLink); //0 is not used

            //remove from channel bus
            i_reqLinkBus->inFlightLink = NULL;

            //remove from SerDes buffer
            i_reqLinkBus->serDesBuffer = NULL;
        }

        bob_linkbus *i_respLinkBus = &respLinkBus[i];
        if(i_respLinkBus->inFlightLinkCountdowns && !--i_respLinkBus->inFlightLinkCountdowns)
        {
            if(i_respLinkBus->serDesBuffer!=NULL)
            {
                ERROR("== Error - Response SerDes Buffer "<<i<<" collision");
                exit(0);
            }

            //note the time
            i_respLinkBus->inFlightLink->channelTimeTotal = currentClockCycle - i_respLinkBus->inFlightLink->channelStartTime;

            //remove from return queue
            BusPacket* front = *channels[i_respLinkBus->inFlightLink->mappedChannel]->readReturnQueue.begin();
            channels[i_respLinkBus->inFlightLink->mappedChannel]->readReturnQueue.erase(channels[i_respLinkBus->inFlightLink->mappedChannel]->readReturnQueue.begin());
            delete front;
            // psiegl ... here in the serdes buffer!
            i_respLinkBus->serDesBuffer = i_respLinkBus->inFlightLink;
            i_respLinkBus->inFlightLink = NULL;
        }
    }

    //
    // NEW STUFF
    //
    for(unsigned c=0; c<NUM_LINK_BUSES; c++)
    {
        //
        //REQUEST
        //
        bob_linkbus *i_reqLinkBus = &reqLinkBus[c];
        if(i_reqLinkBus->serDesBuffer!=NULL &&
           i_reqLinkBus->inFlightLinkCountdowns==0)
        {
            //error checking
            if(i_reqLinkBus->inFlightLink!=NULL)
            {
                ERROR("== Error - item in SerDes buffer without channel being free");
                ERROR("   == Channel : "<<c);
                exit(0);
            }

            //put on channel bus
            i_reqLinkBus->inFlightLink = i_reqLinkBus->serDesBuffer;
            //note the time
            i_reqLinkBus->inFlightLink->channelStartTime = currentClockCycle;

            //the total number of channel cycles the packet will be on the bus
            unsigned totalChannelCycles;
            switch(i_reqLinkBus->inFlightLink->transactionType) {
            case DATA_READ:
                //
                //widths are in bits
                //
                totalChannelCycles = (RD_REQUEST_PACKET_OVERHEAD * 8) / REQUEST_LINK_BUS_WIDTH +
                                     !!((RD_REQUEST_PACKET_OVERHEAD * 8) % REQUEST_LINK_BUS_WIDTH);
                break;
            case DATA_WRITE:
                //
                //widths are in bits
                //
                // inFlightRequestLink[c]->transactionSize == TRANSACTION_SIZE
                totalChannelCycles = ((WR_REQUEST_PACKET_OVERHEAD + i_reqLinkBus->inFlightLink->transactionSize) * 8) / REQUEST_LINK_BUS_WIDTH +
                                     !!(((WR_REQUEST_PACKET_OVERHEAD + i_reqLinkBus->inFlightLink->transactionSize) * 8) % REQUEST_LINK_BUS_WIDTH);
                break;
            case LOGIC_OPERATION:
                //
                //widths are in bits
                //
                totalChannelCycles = (i_reqLinkBus->inFlightLink->transactionSize * 8) / REQUEST_LINK_BUS_WIDTH +
                                     !!((i_reqLinkBus->inFlightLink->transactionSize * 8) % REQUEST_LINK_BUS_WIDTH);
                break;
            default:
                ERROR("== Error - wrong type ");
                exit(0);
            }

            //if the channel uses DDR signaling, the cycles is cut in half
            if(LINK_BUS_USE_DDR)
            {
                totalChannelCycles = totalChannelCycles / 2 + (totalChannelCycles & 0x1);
            }

            //since the channel is faster than the CPU, figure out how many CPU
            //  cycles the channel will be in use
            i_reqLinkBus->inFlightLinkCountdowns = (totalChannelCycles / LINK_CPU_CLK_RATIO) +
                                               !!(totalChannelCycles%LINK_CPU_CLK_RATIO);

            //error check
            if(i_reqLinkBus->inFlightLinkCountdowns==0)
            {
                ERROR("== Error - countdown is 0");
                exit(0);
            }
        }
    }


    //
    //Responses
    //
    for(unsigned p=0; p<this->num_ports; p++)
    {
        if(ports[p].outputBusyCountdown==0)
        {
            //check to see if something has been received from a channel
            bob_linkbus *i_respLinkBus = &respLinkBus[priorityLinkBus[p]];
            if(i_respLinkBus->serDesBuffer!=NULL &&
               i_respLinkBus->serDesBuffer->portID==p)
            {
                i_respLinkBus->serDesBuffer->cyclesRspLink = currentClockCycle - i_respLinkBus->serDesBuffer->cyclesRspLink;
                i_respLinkBus->serDesBuffer->cyclesRspPort = currentClockCycle;
                ports[p].outputBuffer.push_back(i_respLinkBus->serDesBuffer);
                switch(i_respLinkBus->serDesBuffer->transactionType)
                {
                case RETURN_DATA:
                    ports[p].outputBusyCountdown = i_respLinkBus->serDesBuffer->transactionSize / PORT_WIDTH; // serDesBufferResponse[priorityLinkBus[p]]->transactionSize == TRANSACTION_SIZE
                    break;
                case LOGIC_RESPONSE:
                    ports[p].outputBusyCountdown = 1;
                    break;
                default:
                    ERROR("== ERROR - Trying to add wrong type of transaction to output port ");
                    exit(0);
                    break;
                };
                i_respLinkBus->serDesBuffer = NULL;
                // psiegl !! HEEEERE
            }
            priorityLinkBus[p]++;
            if(priorityLinkBus[p]==NUM_LINK_BUSES)priorityLinkBus[p]=0;
        }
    }

    //
    //Move from input ports to serdes
    //
    for(unsigned port=0; port<this->num_ports; port++)
    {
        unsigned p = priorityPort;

        //make sure the port isn't already sending something
        //  and that there is an item there to send
        if(ports[p].inputBusyCountdown==0 &&
           ports[p].inputBuffer.size()>0)
        {
            //search out-of-order
            for(unsigned i=0; i<ports[p].inputBuffer.size(); i++)
            {
                unsigned channelID = FindChannelID(ports[p].inputBuffer[i]);
                unsigned linkBusID = channelID / CHANNELS_PER_LINK_BUS;

                //make sure the serDe isn't busy and the queue isn't full
                bob_linkbus *i_reqLinkBus = &reqLinkBus[linkBusID];
                if(i_reqLinkBus->serDesBuffer==NULL &&
                    channels[channelID]->simpleController.waitingACTS<CHANNEL_WORK_Q_MAX)
                {
                    //put on channel bus
                    i_reqLinkBus->serDesBuffer = ports[p].inputBuffer[i];
                    i_reqLinkBus->serDesBuffer->cyclesReqLink = currentClockCycle;
                    i_reqLinkBus->serDesBuffer->cyclesReqPort = currentClockCycle - i_reqLinkBus->serDesBuffer->cyclesReqPort;
                    i_reqLinkBus->serDesBuffer->mappedChannel = channelID;

                    //keep track of requests
                    channelCounters[channelID]++;

                    switch(i_reqLinkBus->serDesBuffer->transactionType) {
                    case DATA_READ:
                        //put in pending queue
                        //  make it a RETURN_DATA type before we put it in pending queue
                        pendingReads.push_back(ports[p].inputBuffer[i]);

                        readCounter++;

                        //set port busy time
                        ports[p].inputBusyCountdown = 1;
                        break;
                    case DATA_WRITE:
                        writeCounter++;

                        //set port busy time
                        ports[p].inputBusyCountdown = i_reqLinkBus->serDesBuffer->transactionSize / PORT_WIDTH;
                        break;
                    case LOGIC_OPERATION:
//						logicOpCounter++;

                        //set port busy time
                        ports[p].inputBusyCountdown = i_reqLinkBus->serDesBuffer->transactionSize / PORT_WIDTH;
                        break;
                    default:
                        ERROR("== Error - unknown transaction type going to channel : ");
                        exit(0);
                    }

                    priorityPort++;
                    if(priorityPort==this->num_ports) priorityPort = 0;

                    //remove from port input buffer
                    ports[p].inputBuffer.erase(ports[p].inputBuffer.begin()+i);
                    break;
                }
            }
        }

        priorityPort++;
        if(priorityPort==this->num_ports) priorityPort = 0;
    }
    priorityPort++;
    if(priorityPort==this->num_ports) priorityPort = 0;

    for(unsigned link=0; link<NUM_LINK_BUSES; link++)
    {
        //make sure output is not busy sending something else
        bob_linkbus *i_respLinkBus = &respLinkBus[link];
        if(i_respLinkBus->inFlightLinkCountdowns==0 &&
           i_respLinkBus->serDesBuffer==NULL)
        {

            for(unsigned z=0; z<CHANNELS_PER_LINK_BUS; z++)
            {
                unsigned chan = link * CHANNELS_PER_LINK_BUS + responseLinkRoundRobin[link];

                //make sure something is there
                if(channels[chan]->pendingLogicResponse!=NULL)
                {
                    //calculate numbers to see how long the response is on the bus
                    //
                    //widths are in bits
                    unsigned totalChannelCycles = (LOGIC_RESPONSE_PACKET_OVERHEAD * 8) / RESPONSE_LINK_BUS_WIDTH +
                                                  !!((LOGIC_RESPONSE_PACKET_OVERHEAD * 8) % RESPONSE_LINK_BUS_WIDTH);

                    if(LINK_BUS_USE_DDR)
                    {
                        totalChannelCycles = totalChannelCycles / 2 + (totalChannelCycles & 0x1);
                    }

                    //channel countdown
                    i_respLinkBus->inFlightLinkCountdowns = totalChannelCycles / LINK_CPU_CLK_RATIO +
                                                           !!(totalChannelCycles % LINK_CPU_CLK_RATIO);

                    //make sure computation worked
                    if(i_respLinkBus->inFlightLinkCountdowns==0)
                    {
                        ERROR("== ERROR - Countdown 0 on link "<<link);
                        ERROR("==         totalChannelCycles : "<<totalChannelCycles);
                        exit(0);
                    }

                    //make in-flight
                    if(i_respLinkBus->inFlightLink!=NULL)
                    {
                        ERROR("== Error - Trying to set Transaction on down channel while something is there");
                        ERROR("   Cycle : "<<currentClockCycle);
                        ERROR(" Channel : "<<chan);
                        ERROR(" LinkBus : "<<link);
                        exit(0);
                    }

                    //make in flight
                    i_respLinkBus->inFlightLink = channels[chan]->pendingLogicResponse;
                    //remove from channel
                    channels[chan]->pendingLogicResponse = NULL;
                    break;
                }
                else if(channels[chan]->readReturnQueue.size()>0)
                {
                    //remove transaction from pending queue
                    for(unsigned p=0; p<pendingReads.size(); p++)
                    {
                        //find pending item in pending queue
                        if(pendingReads[p]->transactionID == (*channels[chan]->readReturnQueue.begin())->transactionID)
                        {
                            //make the return packet
                            pendingReads[p]->transactionType = RETURN_DATA;

                            //calculate numbers to see how long the response is on the bus
                            //
                            //widths are in bits
                            // pendingReads[p]->transactionSize == TRANSACTION_SIZE
                            unsigned totalChannelCycles = ((RD_RESPONSE_PACKET_OVERHEAD + pendingReads[p]->transactionSize) * 8) / RESPONSE_LINK_BUS_WIDTH +
                                                          !!(((RD_RESPONSE_PACKET_OVERHEAD + pendingReads[p]->transactionSize) * 8) % RESPONSE_LINK_BUS_WIDTH);

                            if(LINK_BUS_USE_DDR)
                            {
                                totalChannelCycles = totalChannelCycles / 2 + (totalChannelCycles & 0x1);
                            }

                            //channel countdown
                            i_respLinkBus->inFlightLinkCountdowns = totalChannelCycles / LINK_CPU_CLK_RATIO
                                                                   + !!(totalChannelCycles % LINK_CPU_CLK_RATIO);

                            //make sure computation worked
                            if(i_respLinkBus->inFlightLinkCountdowns==0)
                            {
                                ERROR("== ERROR - Countdown 0 on link "<<link);
                                exit(0);
                            }

                            //make in-flight
                            if(i_respLinkBus->inFlightLink!=NULL)
                            {
                                ERROR("== Error - Trying to set Transaction on down channel while something is there");
                                ERROR("   Cycle : "<<currentClockCycle);
                                ERROR(" Channel : "<<chan);
                                ERROR(" LinkBus : "<<link);
                                exit(0);
                            }

                            i_respLinkBus->inFlightLink = pendingReads[p];

                            //note the time
                            i_respLinkBus->inFlightLink->cyclesRspLink = currentClockCycle;

                            //remove pending queues
                            pendingReads.erase(pendingReads.begin()+p);

                            //delete channels[chan]->readReturnQueue[0];
                            //channels[chan]->readReturnQueue.erase(channels[chan]->readReturnQueue.begin());
                            break;
                        }
                    }

                    //check to see if we cound an item, and break out of the loop over chans_per_link
                    if(i_respLinkBus->inFlightLinkCountdowns>0)
                    {
                        //increment round robin (increment here to go to next index before we break)
#if CHANNELS_PER_LINK_BUS > 1
                        responseLinkRoundRobin[link]++;
                        if(responseLinkRoundRobin[link]==CHANNELS_PER_LINK_BUS)
#endif
                            responseLinkRoundRobin[link]=0;
                        break;
                    }
                }

                //increment round robin (increment here if we found nothing)
#if CHANNELS_PER_LINK_BUS > 1
                responseLinkRoundRobin[link]++;
                if(responseLinkRoundRobin[link]==CHANNELS_PER_LINK_BUS)
#endif
                    responseLinkRoundRobin[link]=0;
            }
        }
    }
    //
    //Channels update at DRAM speeds (whatever ratio is with CPU)
    //

    if(currentClockCycle>0 && currentClockCycle%DRAM_CPU_CLK_RATIO==0)
    {
        if(DEBUG_CHANNEL) DEBUG(" --------- Channel updates started ---------");

        //if there is an adjustment, we need to increment counter and handle clock differences
        if(DRAM_CPU_CLK_ADJUSTMENT>0)
        {
            if(clockCycleAdjustmentCounter<DRAM_CPU_CLK_ADJUSTMENT)
            {
                dram_channel_clk++;
                for(unsigned i=0; i<NUM_CHANNELS; i++)
                {
                    channels[i]->Update();
                }
            }
            else
              clockCycleAdjustmentCounter=0;

            clockCycleAdjustmentCounter++;
        }
        //if there is no adjustment, just update normally
        else
        {
            for(unsigned i=0; i<NUM_CHANNELS; i++)
            {
                channels[i]->Update();
            }
        }
    }

    //increment clock cycle
    currentClockCycle++;
}

unsigned BOB::FindChannelID(Transaction* trans)
{
#ifndef HMCSIM_SUPPORT
    if(NUM_CHANNELS == 1)
       return 0;

    unsigned channelIDOffset;// = log2(BUS_ALIGNMENT_SIZE) + CHANNEL_ID_OFFSET;
    switch(MAPPINGSCHEME)
    {
    case BK_CLH_RW_RK_CH_CLL_BY://bank:col_high:row:rank:chan:col_low:by
        channelIDOffset = cacheOffset;//bus alignment + column low bits should make the cache offset
        break;
    case CLH_RW_RK_BK_CH_CLL_BY://col_high:row:rank:bank:chan:col_low:byte
        channelIDOffset = cacheOffset;//bus alignment + column low bits should make the cache offset
        break;
    case RK_BK_RW_CLH_CH_CLL_BY://rank:bank:row:col_high:chan:col_low:by
        channelIDOffset = cacheOffset;//bus alignment + column low bits should make the cache offset
        break;
    case RW_BK_RK_CH_CL_BY://row:bank:rank:chan:col:byte
        channelIDOffset = colBitWidth + busOffsetBitWidth;
        break;
    case RW_CH_BK_RK_CL_BY://row:chan:bank:rank:col:byte
        channelIDOffset = busOffsetBitWidth + colBitWidth + rankBitWidth + bankBitWidth;
        break;
    case RW_BK_RK_CLH_CH_CLL_BY://row:bank:rank:col_high:chan:col_low:byte
        channelIDOffset = cacheOffset; //bus alignment + column low bits should make the cache offset
        break;
    case RW_CLH_BK_RK_CH_CLL_BY://row:col_high:bank:rank:chan:col_low:byte
        channelIDOffset = cacheOffset;
        break;
    case CH_RW_BK_RK_CL_BY: //chan:row:bank:rank:col:byte
        channelIDOffset = rowBitWidth + bankBitWidth + rankBitWidth + colBitWidth + busOffsetBitWidth;
        break;
    default:
        ERROR("== ERROR - Unknown address mapping???");
        exit(1);
        break;
    };

    //build channel id mask
    uint64_t channelMask = ((1 << ((unsigned)log2(NUM_CHANNELS)))-1) << channelIDOffset;
    return ((trans->address & channelMask) >> channelIDOffset);
#else
    return 0;
#endif
}

//This is also kind of kludgey, but essentially this function always prints the power
// stats to the output file, but supresses the print to cout until finalPrint is true

void BOB::PrintStats(ofstream &statsOut, ofstream &powerOut, bool finalPrint, unsigned elapsedCycles)
{
    unsigned long dramCyclesElapsed;
    //check if we are on an epoch boundary or if there are cycles left over
    //
    //NOTE : subtract one from currentClockCycle since BOB object Update() has already been called
    //       before this call to PrintStats()

    if((currentClockCycle-1)%EPOCH_LENGTH==0)
    {
        //OLD - doesn't work for non-even ratios
        //dramCyclesElapsed = EPOCH_LENGTH / DRAM_CPU_CLK_RATIO;

        //NEW
        dramCyclesElapsed = EPOCH_LENGTH / ((float)tCK/(float)CPU_CLK_PERIOD);
    }
    else
    {
        //same as above
        dramCyclesElapsed = ((currentClockCycle-1)%EPOCH_LENGTH)/((float)tCK/(float)CPU_CLK_PERIOD);
    }

    //dramCyclesElapsed = channels[0]->currentClockCycle;

#define MAX_TMP_STR 512
    char tmp_str[MAX_TMP_STR];

    PRINT(std::dec << " === BOB Print === ");
    unsigned long totalRequestsAtChannels = 0L;
    PRINT(" == Ports");
    for(unsigned p=0; p<this->num_ports; p++)
    {
        PRINT("  -- Port "<<p<<" - inputBufferAvg : "<<(float)portInputBufferAvg[p]/elapsedCycles<<" ("<<ports[p].inputBuffer.size()<<")   outputBufferAvg : "<<(float)portOutputBufferAvg[p]/elapsedCycles<<" ("<<ports[p].outputBuffer.size()<<")");
        portInputBufferAvg[p]=0;
        portOutputBufferAvg[p]=0;
    }

    //calculate possible bandwidth of part
    float dataperiod = tCK/2;
    float bw = (1/dataperiod)*64/8;//64-bit bus, 8 bits/byte

#ifndef NO_OUTPUT
    unsigned long long totalBytesPerDevice = (unsigned long long)NUM_COLS * (unsigned long long)NUM_ROWS * (unsigned long long)NUM_BANKS; //in bytes
    unsigned long long gigabytesPerRank = totalBytesPerDevice>>20;
#endif

    //calculate peak bandwidth for link bus
    float reqbwpeak = (REQUEST_LINK_BUS_WIDTH / LINK_BUS_CLK_PERIOD) / 8; //in bytes
    float rspbwpeak = (RESPONSE_LINK_BUS_WIDTH / LINK_BUS_CLK_PERIOD) / 8; //in bytes
    if(LINK_BUS_USE_DDR)
    {
        reqbwpeak *= 2;
        rspbwpeak *= 2;
    }

    PRINT(" == Link Bandwidth (!! Includes packet overhead and request packets !!)");
    PRINT("    Req Link("<<reqbwpeak<<" GB/s peak)  Rsp Link("<<rspbwpeak<<" GB/s peak)");
    double reqtotal = 0;
    double rsptotal = 0;
    for(int l=0; l<NUM_LINK_BUSES; l++)
    {
        reqtotal += (1-(float)reqLinkBus[l].linkIdle/(float)elapsedCycles) * reqbwpeak;
        rsptotal += (1-(float)respLinkBus[l].linkIdle/(float)elapsedCycles) * rspbwpeak;
        PRINTN("      "<<(1-(float)reqLinkBus[l].linkIdle/(float)elapsedCycles) * reqbwpeak<<" GB/s             ");
        reqLinkBus[l].linkIdle=0;
        PRINT((1-(float)respLinkBus[l].linkIdle/(float)elapsedCycles) * rspbwpeak<<" GB/s");
        respLinkBus[l].linkIdle=0;
    }

#if NUM_LINK_BUSES > 1
    PRINT("     -----------");
    PRINT("      "<<reqtotal/NUM_LINK_BUSES<<"          "<<rsptotal/NUM_LINK_BUSES<<" (avgs)");
#endif

    PRINT(" == Channel Usage and Stats ("<<(this->num_ranks * gigabytesPerRank)<<"MB/Chan == "<<this->num_ranks * gigabytesPerRank * NUM_CHANNELS<<" MB total)");
    PRINT("     reqs   workQAvg  workQMax idleBanks   actBanks  preBanks  refBanks  (totalBanks) BusIdle  BW("<<bw<<")  RRQMax("<<CHANNEL_RETURN_Q_MAX/TRANSACTION_SIZE<<")   RRQFull lifetimeRequests");
    float totalDRAMbw = 0;
    for(unsigned i=0; i<NUM_CHANNELS; i++)
    {
        //compute each DRAM channel's BW
        double DRAMBandwidth= (bw * (1 - ((double)channels[i]->DRAMBusIdleCount/(double)dramCyclesElapsed))) * 1E9 / (1<<30);

        statsOut<<DRAMBandwidth<<",";

        totalDRAMbw += DRAMBandwidth;
        totalRequestsAtChannels+=channelCounters[i];
        channelCountersLifetime[i]+=channelCounters[i];

        // since trying to actually format strings with stream operators is a huge pain
        snprintf(tmp_str, MAX_TMP_STR, "%u]%9u%10.4f%10u%10.4f%10.4f%10.4f%10.4f%10.4f%10.2f%10.3f%10u(%d)%10u%10llu\n",
                 i,
                 channelCounters[i],
                 (float)channels[i]->simpleController.commandQueueAverage/dramCyclesElapsed,
                 channels[i]->simpleController.commandQueueMax,
                 (float)channels[i]->simpleController.numIdleBanksAverage/dramCyclesElapsed,
                 (float)channels[i]->simpleController.numActBanksAverage/dramCyclesElapsed,
                 (float)channels[i]->simpleController.numPreBanksAverage/dramCyclesElapsed,
                 (float)channels[i]->simpleController.numRefBanksAverage/dramCyclesElapsed,
                 (float)(channels[i]->simpleController.numIdleBanksAverage+
                         channels[i]->simpleController.numActBanksAverage+
                         channels[i]->simpleController.numPreBanksAverage+
                         channels[i]->simpleController.numRefBanksAverage)/dramCyclesElapsed,
                 100*((float)channels[i]->DRAMBusIdleCount/(float)dramCyclesElapsed),
                 DRAMBandwidth,
                 channels[i]->readReturnQueueMax,
                 (int)channels[i]->readReturnQueue.size(),
                 channels[i]->simpleController.RRQFull,
                 (unsigned long long)channelCountersLifetime[i]
                );

//        for(int r=0; r<this->num_ranks; r++)
//        {
//            for(int b=0; b<NUM_BANKS; b++)
//            {
                //PRINTN(channels[i]->simpleController.bankStates[r][b]);
//            }
//        }

        //reset
        channels[i]->simpleController.commandQueueAverage=0;
        channels[i]->simpleController.numIdleBanksAverage=0;
        channels[i]->simpleController.numActBanksAverage=0;
        channels[i]->simpleController.numPreBanksAverage=0;
        channels[i]->simpleController.numRefBanksAverage=0;
        channels[i]->simpleController.commandQueueMax=0;
        channels[i]->readReturnQueueMax=0;
        channels[i]->DRAMBusIdleCount=0;
        channelCounters[i]=0;
        channels[i]->simpleController.RRQFull=0;

        PRINTN(tmp_str);
    }

    //separates bandwidth numbers and power numbers
    statsOut<<";";

#if NUM_CHANNELS > 1
    PRINT("                                                                                          AVG : "<<totalDRAMbw/NUM_CHANNELS);
#endif
    PRINT(" == Requests seen at Channels");
    PRINT("  -- Reads  : "<<readCounter);
    PRINT("  -- Writes : "<<writeCounter);
    PRINT("            = "<<totalRequestsAtChannels);
    readCounter = 0;
    writeCounter = 0;

    //
    //
    //POWER
    //
    //
    PRINT(" == Channel Power");
    powerOut<<currentClockCycle * CPU_CLK_PERIOD / 1000000<<",";

    bool detailedOutput = true;
    bool shortOutput = false;
    float allChanAveragePower = 0;
    for(unsigned c=0; c<NUM_CHANNELS; c++)
    {
        float totalChannelPower = 0;
#ifndef HMCSIM_SUPPORT
        PRINTN("    -- Channel "<<c);
#endif
        for(unsigned r=0; r<this->num_ranks; r++)
        {
            float averagePower = ((float)(channels[c]->simpleController.actpreEnergy[r] +
                                          channels[c]->simpleController.backgroundEnergy[r] +
                                          channels[c]->simpleController.burstEnergy[r] +
                                          channels[c]->simpleController.refreshEnergy[r]) / (float) dramCyclesElapsed) * Vdd / 1000;
            totalChannelPower += averagePower;

            if(!shortOutput)
            {
                PRINTN("    -- Rank "<<r<<": ");
                if(detailedOutput)
                {
#ifndef NO_OUTPUT
                    float backgroundPower = ((float)channels[c]->simpleController.backgroundEnergy[r] / (float) dramCyclesElapsed) * Vdd / 1000;
                    float burstPower = ((float)channels[c]->simpleController.burstEnergy[r] / (float) dramCyclesElapsed) * Vdd / 1000;
                    float actprePower = ((float)channels[c]->simpleController.actpreEnergy[r] / (float) dramCyclesElapsed) * Vdd / 1000;
                    float refreshPower = ((float)channels[c]->simpleController.refreshEnergy[r] / (float) dramCyclesElapsed) * Vdd / 1000;
#endif

                    PRINT(setprecision(4)<<"TOT :"<<averagePower<<"  bkg:"<<backgroundPower<<" brst:"<<burstPower<<" ap:"<<actprePower<<" ref:"<<refreshPower << setprecision(6));
                }
                else
                {
                    PRINTN(setprecision(4)<<averagePower<<"w " << setprecision(6));
                }
            }

            //clear for next epoch
            channels[c]->simpleController.backgroundEnergy[r]=0;
            channels[c]->simpleController.burstEnergy[r]=0;
            channels[c]->simpleController.actpreEnergy[r]=0;
            channels[c]->simpleController.refreshEnergy[r]=0;
        }

        allChanAveragePower += totalChannelPower;
        statsOut<<totalChannelPower<<",";
        powerOut<<totalChannelPower<<",";

        PRINT("    -- DRAM Power : "<<totalChannelPower<<" w");
    }
    statsOut << ";" << allChanAveragePower/NUM_CHANNELS << endl;

#if NUM_CHANNELS > 1
    PRINT("   Average Power  : "<<allChanAveragePower/NUM_CHANNELS<<" w");
    PRINT("   Total Power    : "<<allChanAveragePower<<" w");
#endif
#ifndef HMCSIM_SUPPORT // ToDo: check in HMCSIM spec, if same info is available!
    PRINT("   SimpCont BG Power : "<<NUM_LINK_BUSES * SIMP_CONT_BACKGROUND_POWER<<" w");
    PRINT("   SimpCont Core Power : "<<NUM_CHANNELS * SIMP_CONT_CORE_POWER<<" w");
    PRINT("   System Power   : "<<allChanAveragePower + NUM_LINK_BUSES * SIMP_CONT_BACKGROUND_POWER + NUM_CHANNELS * SIMP_CONT_CORE_POWER<<" w");
#endif

    //compute static power from controllers
    powerOut<<SIMP_CONT_BACKGROUND_POWER * NUM_LINK_BUSES + NUM_CHANNELS * SIMP_CONT_CORE_POWER <<",";
    powerOut<<(SIMP_CONT_BACKGROUND_POWER * NUM_LINK_BUSES + NUM_CHANNELS * SIMP_CONT_CORE_POWER) + allChanAveragePower<<endl;

    PRINT(" == Time Check");
    PRINT("    CPU Time : "<<currentClockCycle * CPU_CLK_PERIOD<<"ns");
    PRINT("   DRAM Time : "<<(dram_channel_clk * tCK)<<"ns");
}


//
//Callback shit
//
void BOB::ReportCallback(BusPacket *bp)
{
    switch(bp->busPacketType) {
    case ACTIVATE:
        for(unsigned p=0; p<pendingReads.size(); p++)
        {
            if(pendingReads[p]->transactionID==bp->transactionID)
            {
                pendingReads[p]->dramStartTime = currentClockCycle;
                pendingReads[p]->cyclesInWorkQueue=bp->queueWaitTime;
                break;
            }
        }
        break;
    case WRITE_P:
        bobwrapper->WriteIssuedCallback(bp->port, bp->address);
        break;
    case WRITE_DATA:
        committedWrites++;
        break;
    case READ_DATA:
        for(unsigned p=0; p<pendingReads.size(); p++)
        {
            if(pendingReads[p]->transactionID==bp->transactionID)
            {
                pendingReads[p]->dramTimeTotal = currentClockCycle - pendingReads[p]->dramStartTime;
                break;
            }
        }
        break;
    default:
        ERROR("== Error - Wrong type of packet received in bob");
        exit(0);
    }
}

} //namespace BOBSim


