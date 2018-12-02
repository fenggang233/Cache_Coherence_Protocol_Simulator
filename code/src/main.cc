/**
 * \file main.cc
 * \author NCSU ECE 506 TA (2018), Soumil Krishnanand Heble
 * \date 11/29/2018
 * \brief Source: Cache Coherence Protocol Simulator
 */

#include <stdlib.h>
#include <assert.h>
#include <fstream>
using namespace std;

#include "cache.h"
#include "coherence_ctrl.h"

int main(int argc, char *argv[])
{
	
	ifstream fin;
	FILE *pFile;
        
        enum coh_protocol currentProtocol;

	if(argv[1] == NULL)
        {
		 printf("input format: ");
		 printf("./smp_cache <cache_size> <assoc> <block_size> <num_processors> <protocol> <trace_file> \n");
		 exit(0);
        }

	int cache_size = atoi(argv[1]);
	int cache_assoc= atoi(argv[2]);
	int blk_size   = atoi(argv[3]);
	int num_processors = atoi(argv[4]);/*1, 2, 4, 8*/
	int protocol   = atoi(argv[5]);	 /*0:MSI, 1:MESI, 2:Dragon*/
	char *fname =  (char *)malloc(20);
 	fname = argv[6];

        /** Personal Information to stdout */
	printf("===== 506 Personal information =====\n");
        printf("Soumil Krishnanand Heble\n");
        printf("sheble\n");
        printf("ECE492 Students? NO\n");
        
        switch(protocol)
        {
            case 0: currentProtocol = MSI;
                    break;
                    
            case 1: currentProtocol = MESI;
                    break;
                    
            case 2: currentProtocol = DRAGON;
                    break;
                    
            default:    printf("COHERENCE PROTOCOL: UNKNOWN, Wrong Argument\n");
                        exit(0);
        }

        /** Create Coherence Controller Class Object Here with Constructor */
        coherenceController simController = coherenceController(cache_size, cache_assoc, blk_size, num_processors, currentProtocol);

	pFile = fopen (fname,"r");
	if(pFile == 0)
	{   
		printf("Trace file problem\n");
		exit(0);
	}
	
	/** Print Simulation Parameters and Cache Specifications */
	printf("===== 506 SMP Simulator configuration =====\n");
        printf("L1_SIZE: %d\n", cache_size);
        printf("L1_ASSOC: %d\n", cache_assoc);
        printf("L1_BLOCKSIZE: %d\n", blk_size);
        printf("NUMBER OF PROCESSORS: %d\n", num_processors);
        
        switch(protocol)
        {
            case 0: printf("COHERENCE PROTOCOL: MSI\n");
                    break;
                    
            case 1: printf("COHERENCE PROTOCOL: MESI\n");
                    break;
                    
            case 2: printf("COHERENCE PROTOCOL: Dragon\n");
                    break;
                    
            default:    printf("COHERENCE PROTOCOL: UNKNOWN, Wrong Argument\n");
        }
        printf("TRACE FILE: %s\n", fname);
        
        /** File Read Storage Variables */
        char str[2];
        unsigned long int procNum;
        unsigned long int procReqAddr;
        unsigned char reqRW;
        
        /** Read File Line By Line Until EOF */
        while(fscanf(pFile, "%lu %s %lx", &procNum, str, &procReqAddr) != EOF)
        {
            if(str[0]=='w')
            {
                reqRW = (unsigned char)1;
            }
            else
            {
                reqRW = (unsigned char)0;
            }
            
            /** Call the Coherence Controller Class Object with the processAddress method */
            simController.processRequest(procNum, reqRW, procReqAddr);
            
            /** Debug: File Read Print */
            //printf("%lu %c %lx\n", procNum, str[0], procReqAddr);
        }
        
	fclose(pFile);

	/** Call the Coherence Controller Class Object with the dumpData method */
        simController.dumpMetrics();
}
