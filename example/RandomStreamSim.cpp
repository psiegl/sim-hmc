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

//TraceBasedSim.cpp
//
//File to run a trace-based simulation
//

#include <iostream>
#include <getopt.h>
#include <map>
#include <cstdio>
#include <cstdlib>
#include <deque>
#include "../include/bob_globals.h"
#include "../include/bob_transaction.h"
#include "../include/bob.h"
#include "../include/bob_wrapper.h"
#include "../include/bob_logiclayerinterface.h"
#include "../include/bob_logicoperation.h"



//Total number of reads (as a percentage) in request stream
#define READ_WRITE_RATIO 66.6f //%
//Frequency of requests - 0.0 = no requests; 1.0 = as fast as possible
#define PORT_UTILIZATION  1.0f

//Changes random seeding for request stream
#define SEED_CONSTANT 11


using namespace BOBSim;
using namespace std;

int BOBSim::SHOW_SIM_OUTPUT=1;

vector<unsigned> waitCounters;
vector<unsigned> useCounters;

void usage()
{
	cout << "Blah" << endl;
}

vector< vector<Transaction *> > transactionBuffer;

void FillTransactionBuffer(int port)
{
	useCounters[port] = 0;

	//Fill with a random number of requests between 2 and 5 or 6
    unsigned randomCount = rand() % 5 + 2;
	//cout<<"port "<<port<<" - random count is "<<randomCount<<endl;
	for(unsigned i=0; i<randomCount; i++)
	{
		unsigned long temp = rand();
        unsigned long physicalAddress = (temp<<32)|rand();
		Transaction *newTrans;
		if(physicalAddress%1000<READ_WRITE_RATIO*10)
		{
            newTrans = new Transaction(DATA_READ,physicalAddress,TRANSACTION_SIZE);
			//cout<<*newTrans<<endl;
			useCounters[port]+= RD_REQUEST_PACKET_OVERHEAD/PORT_WIDTH+!!(RD_REQUEST_PACKET_OVERHEAD%PORT_WIDTH);
		}
		else
		{
            newTrans = new Transaction(DATA_WRITE,physicalAddress,TRANSACTION_SIZE);
			//cout<<*newTrans<<endl;
			useCounters[port]+=(WR_REQUEST_PACKET_OVERHEAD + TRANSACTION_SIZE)/PORT_WIDTH +
				!!((WR_REQUEST_PACKET_OVERHEAD + TRANSACTION_SIZE)%PORT_WIDTH);
		}

        transactionBuffer[port].push_back(newTrans);
	}
	
	//cout<<"use for "<<port<<" is "<<useCounters[port]<<endl;
	//figure out how much idle time between requests
	waitCounters[port] = ceil(useCounters[port]/PORT_UTILIZATION) - useCounters[port];
	//cout<<"wait for "<<port<<" is "<<waitCounters[port]<<endl;
}

int main(int argc, char **argv)
{
    long numCycles=30l;
    srand(SEED_CONSTANT);
    unsigned num_ports = 1;
	while (1)
	{
		static struct option long_options[] =
        {
			{"numcycles",  required_argument,	0, 'c'},
			{"quiet",  no_argument, &BOBSim::SHOW_SIM_OUTPUT, 'q'},
			{"help", no_argument, 0, 'h'},
			{0, 0, 0, 0}
		};
		int option_index=0; //for getopt
        int c = getopt_long (argc, argv, "c:n:q", long_options, &option_index);
		if (c == -1)
		{
			break;
		}
		switch (c)
        {
		case 'h':
		case '?':
			usage();
			exit(0);
			break;
		case 'c':
			numCycles = atol(optarg);
			break;
		case 'n':
            num_ports = atoi(optarg);
		case 'q':
			BOBSim::SHOW_SIM_OUTPUT=0;
			break;
		}
    }

    transactionBuffer = vector< vector<Transaction *> >(num_ports,vector<Transaction *>());

    waitCounters = vector<unsigned>(num_ports,0);
    useCounters = vector<unsigned>(num_ports,0);


	//iterate over total number of cycles
	//  "main loop"
	//   numCycles is the number of CPU cycles to simulate
    BOBWrapper *bobWrapper = new BOBWrapper(num_ports);
	for (int cpuCycle=0; cpuCycle<numCycles; cpuCycle++)
	{
        //adding new stuff
        for(unsigned l=0; l<num_ports; l++)
		{
			if(transactionBuffer[l].size()>0)
			{
                if(bobWrapper->AddTransaction(*transactionBuffer[l].begin(),l))
                {
					transactionBuffer[l].erase(transactionBuffer[l].begin());
				}
			}
			else
            {
                waitCounters[l] -= (waitCounters[l]>0);

                //make sure we are not waiting during idle time
                if(waitCounters[l]==0)
                {
                    FillTransactionBuffer(l);
                }
			}
		}

		//
		//Update bobWrapper
		//
        bobWrapper->Update();
	}

    // remove not required transactions
    for(unsigned l=0; l<num_ports; l++)
    {
      for(vector<Transaction *>::iterator it = transactionBuffer[l].begin(); it != transactionBuffer[l].end(); ++it)
      {
        delete *it;
      }
    }
	//
	//Debug output
	//
    bobWrapper->PrintStats(true);
    delete bobWrapper;
}
