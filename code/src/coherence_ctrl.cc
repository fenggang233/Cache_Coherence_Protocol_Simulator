/**
 * \file coherence_ctrl.cc
 * \author Soumil Krishnanand Heble
 * \date 12/01/2018
 * \brief Source: Coherence Controller and Bus Class
 */

#include "coherence_ctrl.h"
#include <assert.h>

#define RD_REQ 0
#define WR_REQ 1

coherenceController::coherenceController(int s, int a, int b, int numP, enum coh_protocol cohProtocol)
{
    num_processors = numP;
    coherenceProtocol = cohProtocol;
    busControl = 0xFF;
    busValid = VALID_BUS;
    busAddr = 0;
    busData = 0;
    copiesExist = NCEX;
    busCommand = INVALID_BUS;
    hitMiss = MISS;
    
    cacheOnbus = new Cache*[numP];
    
    uchar loop_i;
    for(loop_i=0;loop_i<numP;loop_i++)
    {
        cacheOnbus[loop_i] = new Cache(s, a, b);
    }
}

void coherenceController::processRequest(ulong procNum, uchar rdWr, ulong reqAddr)
{
    switch(coherenceProtocol)
    {
        case MSI:   processMSI(procNum, rdWr, reqAddr);
                    break;
                    
        case MESI:  processMESI(procNum, rdWr, reqAddr);
                    break;
                    
        case DRAGON:    processDRAGON(procNum, rdWr, reqAddr);
                        break;
    }
}

void coherenceController::processMSI(ulong procNum, uchar rdWr, ulong reqAddr)
{
    /** Increment the Cache's Request Count */
    cacheOnbus[procNum]->inccurrentCycle();
    
    /** Pointer to Cache Line to Update Finally */
    cacheLine *line;
    
    /** Check Type of Read and Update Counter */
    if(rdWr==RD_REQ)
    {
        cacheOnbus[procNum]->incReads();
    }
    else
    {
        cacheOnbus[procNum]->incWrites();
    }
    
    /** Look for the Requested Address in the Cache */
    line = cacheOnbus[procNum]->findLine(reqAddr);
    
    /** If Cache Line Not Found */
    if(line==NULL)
    {
        /** Set To Miss */
        hitMiss = MISS;
        
        /** Fetch a Victim Cache Line for the Data */
        cacheLine *victim = cacheOnbus[procNum]->findLineToReplace(reqAddr);
        assert(victim != 0);
        
        /** Store Victim Block to the Cache Line to Update Pointer */
        line = victim;
        
        if(victim->getFlags()==MODIFIED)
        {
            /** Update Writeback Counter */
            cacheOnbus[procNum]->incWB();
            
            /** Update Memory Transaction Counter */
            cacheOnbus[procNum]->incMemtransactions();
        }
        
        /* Bus in Control of Current Processor */
        busControl = procNum;
        
        /* Bus Signals are Valid */
        busValid = VALID_BUS;
        
        /* Place Required Address on the Bus */
        busAddr = reqAddr;
        
        if(rdWr==RD_REQ)
        {
            /** Update Read Miss Counters */
            cacheOnbus[procNum]->incRM();
            
            /** Place BusRd Command on Bus */
            busCommand = BUSRD;
            
            /** Update BusRd Counter */
            cacheOnbus[procNum]->incBusrd();
        }
        else
        {
            /** Update Write Miss Counters */
            cacheOnbus[procNum]->incWM();
            
            /** Place BusRdX Command on Bus */
            busCommand = BUSRDX;
            
            /** Update BusRdX Counter */
            cacheOnbus[procNum]->incBusrdx();
        }
    }
    else
    {
        /** Set To Hit */
        hitMiss = HIT;
        
        /** Record Initial Flag of the Cache Line */
//         initialFlag = line->getFlags();
        
        if((line->getFlags()==SHARED)&&(rdWr==WR_REQ))
        {
            /* Bus in Control of Current Processor */
            busControl = procNum;
            
            /* Bus Signals are Valid */
            busValid = VALID_BUS;
            
            /* Place Required Address on the Bus */
            busAddr = reqAddr;
            
            /** Place BusRdX Command on Bus */
            busCommand = BUSRDX;
            
            /** Update BusRdX Counter */
            cacheOnbus[procNum]->incBusrdx();
        }
    }
    
    /* Perform Bus Snooping Operations */
    if(busValid==VALID_BUS)
    {
        uchar loop_i;
        
        switch(busCommand)
        {
            case BUSRD: cacheOnbus[procNum]->incMemtransactions();
                        for(loop_i=0; loop_i<num_processors; loop_i++)
                        {
                            /** Look for the Request Address in the Cache */
                            cacheLine *line_procn = cacheOnbus[loop_i]->findLine(busAddr);
                            
                            /** If Found in Cache Other than Cache Serivicing the Request */
                            if((line_procn!=NULL)&&(loop_i!=procNum)&&(line_procn->getFlags()==MODIFIED))
                            {
                                /** Set Cache Line State To Shared */
                                line_procn->setFlags(SHARED);
                                
                                /** Update Intervention Counter */
                                cacheOnbus[loop_i]->incInterv();
                                
                                /** Update Flush Counter */
                                cacheOnbus[loop_i]->incFlush();
                                
                                cacheOnbus[loop_i]->incMemtransactions();
                                
                                /** Update Writeback Counter */
                                cacheOnbus[loop_i]->incWB();
                                
                                busControl = loop_i;
                                busCommand = FLUSH;
                            }
                        }
                        break;
                        
            case BUSRDX:    cacheOnbus[procNum]->incMemtransactions();
                            for(loop_i=0; loop_i<num_processors; loop_i++)
                            {
                                /** Look for the Request Address in the Cache */
                                cacheLine *line_proc = cacheOnbus[loop_i]->findLine(busAddr);
                                
                                /** If Found in Cache Other than Cache Serivicing the Request */
                                if((line_proc!=NULL)&&(loop_i!=procNum))
                                {
                                    /** Update Invalidation Counter */
                                    cacheOnbus[loop_i]->incInval();
                                    
                                    /** If Modified */
                                    if(line_proc->getFlags()==MODIFIED)
                                    {
                                        /** Update Flush Counter */
                                        cacheOnbus[loop_i]->incFlush();
                                        
                                        cacheOnbus[loop_i]->incMemtransactions();
                                        
                                        /** Update Writeback Counter */
                                        cacheOnbus[loop_i]->incWB();
                                        
                                        busControl = loop_i;
                                        busCommand = FLUSH;
                                    }
                                    
                                    /** Invalidate Cache Line */
                                    line_proc->invalidate();
                                }
                            }
                            break;
            
            default:    printf("ERROR\n");
                        break;
        }
    }
    
    /** Perform Finishing Actions */
    if(hitMiss==HIT)
    {
        //ulong tag;
        //tag = cacheOnbus[procNum]->calcTag(reqAddr);
        //line->setTag(tag);
        
        cacheOnbus[procNum]->updateLRU(line);
        
        if(rdWr==WR_REQ)
        {
            line->setFlags(MODIFIED);
//             finalFlag = MODIFIED;
        }
//         else
//         {
//             finalFlag = SHARED;
//         }
    }
    else
    {
        ulong tag;
        tag = cacheOnbus[procNum]->calcTag(reqAddr);
        line->setTag(tag);
        
        cacheOnbus[procNum]->updateLRU(line);
        
        if(rdWr==WR_REQ)
        {
            line->setFlags(MODIFIED);
//             finalFlag = MODIFIED;
        }
        else
        {
            line->setFlags(SHARED);
//             finalFlag = SHARED;
        }
    }
    
    /** Reset Bus */
    busControl = 0xFF;
    busValid = INVALID_BUS;
    busAddr = 0xFFFFFFFF;
    busCommand = INVALID_BUS;
    
    hitMiss = RST_OUT;
    
    /** Update Invalidation or Intervention Counter */
//     if(finalFlag==INVALID)  /** It's Invalidation */
//     {
//         cacheOnbus[procNum]->incInval();
//     }
//     else
//     if((initialFlag==MODIFIED)&&(finalFlag==SHARED))    /** It's Intervention */
//     {
//         cacheOnbus[procNum]->incInterv();
//     }
}

void coherenceController::processMESI(ulong procNum, uchar rdWr, ulong reqAddr)
{
    printf("ERROR: MESI\n");
}

void coherenceController::processDRAGON(ulong procNum, uchar rdWr, ulong reqAddr)
{
    printf("ERROR: DRAGON\n");
}

void coherenceController::dumpMetrics()
{
    uchar loop_i;
    for(loop_i=0; loop_i<num_processors; loop_i++)
    {
        
        printf("============ Simulation results (Cache %u) ============\n",(uint)loop_i);
        printf("01. number of reads:%34lu\n", cacheOnbus[loop_i]->getReads());
        printf("02. number of read misses:%26lu\n", cacheOnbus[loop_i]->getRM());
        printf("03. number of writes:%32lu\n", cacheOnbus[loop_i]->getWrites());
        printf("04. number of write misses:%23lu\n", cacheOnbus[loop_i]->getWM());
        printf("05. total miss rate:%32.2f%%\n", ((float)(cacheOnbus[loop_i]->getRM()+cacheOnbus[loop_i]->getWM()))*100.0/((float)(cacheOnbus[loop_i]->getReads()+cacheOnbus[loop_i]->getWrites())));
        printf("06. number of writebacks:%26lu\n", cacheOnbus[loop_i]->getWB());
        printf("07. number of cache-to-cache transfers:%10lu\n", cacheOnbus[loop_i]->getCache2cache());
        printf("08. number of memory transactions:%19lu\n", cacheOnbus[loop_i]->getMemtransactions());
        printf("09. number of interventions:%22lu\n", cacheOnbus[loop_i]->getInterv());
        printf("10. number of invalidations:%24lu\n", cacheOnbus[loop_i]->getInval());
        printf("11. number of flushes:%28lu\n", cacheOnbus[loop_i]->getFlush());
        printf("12. number of BusRdX:%30lu\n", cacheOnbus[loop_i]->getBusrdx());
        
        /*
        printf("============ Simulation results (Cache %u) ============\n",(uint)loop_i);
        printf("01. number of reads:%lu\n", cacheOnbus[loop_i]->getReads());
        printf("02. number of read misses:%lu\n", cacheOnbus[loop_i]->getRM());
        printf("03. number of writes:%lu\n", cacheOnbus[loop_i]->getWrites());
        printf("04. number of write misses:%lu\n", cacheOnbus[loop_i]->getWM());
        printf("05. total miss rate:%f\n", ((float)(cacheOnbus[loop_i]->getRM()+cacheOnbus[loop_i]->getWM()))*100.0/((float)(cacheOnbus[loop_i]->getReads()+cacheOnbus[loop_i]->getWrites())));
        printf("06. number of writebacks:%lu\n", cacheOnbus[loop_i]->getWB());
        printf("07. number of cache-to-cache transfers:%lu\n", cacheOnbus[loop_i]->getCache2cache());
        printf("08. number of memory transactions:%lu\n", cacheOnbus[loop_i]->getMemtransactions());
        printf("09. number of interventions:%lu\n", cacheOnbus[loop_i]->getInterv());
        printf("10. number of invalidations:%lu\n", cacheOnbus[loop_i]->getInval());
        printf("11. number of flushes:%lu\n", cacheOnbus[loop_i]->getFlush());
        printf("12. number of BusRdX:%lu\n", cacheOnbus[loop_i]->getBusrdx());
        */
    }
}
