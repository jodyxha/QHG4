#include <cstdio>

#include "xPopBase.h"
#include "xPopLooper.h"
#include "OccHistory.h"
#include "OccAnalyzer.h"
#include "OccTracker.h"

#define NUM_CELLS 5
#define NUM_TIMES 6
int gaga1[NUM_TIMES][NUM_CELLS] = {
    {1,0,0,0,0},
    {1,1,0,0,0},
    {1,0,0,0,0},
    {0,0,0,0,0},
    {0,0,0,0,0},
    {0,0,0,0,0}
};

int nunu1[NUM_TIMES][NUM_CELLS] = {
    {0,0,0,0,1},
    {0,1,0,1,1},
    {0,0,0,1,1},
    {0,0,1,1,1},
    {0,1,1,1,1},
    {1,1,1,1,1}
};

int rere1[NUM_TIMES][NUM_CELLS] = {
    {0,0,1,0,0},
    {0,0,1,1,0},
    {0,1,1,0,1},
    {0,1,0,0,0},
    {1,0,0,0,0},
    {0,0,0,0,0}
};

int main(int iArgC, char *apArgV[]) {
    int iResult = 0;

    int iNumCells = NUM_CELLS;
    xPopLooper *pPL = new xPopLooper;
    xPopBase *pGaga = new xPopBase("gaga", iNumCells);

    pPL->addPop(pGaga);
    xPopBase *pNunu = new xPopBase("nunu", iNumCells);
    pPL->addPop(pNunu);
    xPopBase *pRere = new xPopBase("rere", iNumCells);
    pPL->addPop(pRere);;

    std::vector<int> vCellIDs;
    for (int i = 0; i < iNumCells; i++) {
        vCellIDs.push_back(i);
    }
    OccTracker *pOT = OccTracker::createInstance(vCellIDs, pPL);
    if (pOT != NULL) {
        
        for (int i = 0; i < NUM_TIMES; i++) {
            pGaga->setNumAgents(gaga1[i], iNumCells);
            pNunu->setNumAgents(nunu1[i], iNumCells);
            pRere->setNumAgents(rere1[i], iNumCells);
            pOT->updateCounts(i);
        }
        //  pOC->finalizeCounts();

        for (int i = 0; i < NUM_CELLS; i++) {
            printf("---  cedll %d ---\n", i);
            pOC->showHistory(i);
        }
#define EPS 1e-5
        uint *pVals = new uint[iNumCells];
        for (int x = 0; x<=NUM_TIMES;x++) {
            uint *po = pOC->occupationAtTime(x+EPS, pVals);
            printf("T %f\n", x*1.0);
            for (int i = 0; i < NUM_CELLS; i++) {
                printf(" C %d %08x\n", i, po[i]);
            }
        }
        delete[] pVals;
        
        printf("--- region history ---\n");
        std::vector<int> vSubCellIDs;
        for (int i = 0; i < NUM_CELLS; i++) {
            if (i < 3)  {
            vSubCellIDs.push_back(i);
            }
        }
        timed_bits tb;
        pOC->occupationOfRegion(vSubCellIDs, tb);
        pOC->showHistory(tb);
        
        uchar *p1 = pOC->serialize();
        std::vector<int> vEmpty;

        xPopLooper *pPL2 = new xPopLooper;
 
        xPopBase *pToto = new xPopBase("toto", iNumCells);
        pPL2->addPop(pToto);
        pPL2->addPop(pGaga);
        xPopBase *pSisi = new xPopBase("sisi", iNumCells);
        pPL2->addPop(pSisi);;

        xPopLooper *pPL3 = new xPopLooper;
 
        pPL3->addPop(pNunu);
        pPL3->addPop(pRere);
        pPL3->addPop(pGaga);

        /*
        OccChecker *pOC2 = OccChecker::createInstance(vEmpty, pPL2);
        iResult = pOC2->deserialize(p1);
        if (iResult != 0) {
            delete pOC2;
        */
             OccChecker *pOC2 = OccChecker::createInstance(vEmpty, pPL3);
            iResult = pOC2->deserialize(p1);

            //}
        printf("history 2\n");
        pOC2->occupationOfRegion(vSubCellIDs, tb);
        pOC2->showHistory(tb);
        pOC2->occupationOfRegion(vCellIDs, tb);
        pOC2->showHistory(tb);
        delete pOC2;
        delete pOC;
        delete pToto;
        delete pSisi;
        delete pGaga;
        delete pNunu;
        delete pRere;
        delete pPL;
        delete pPL2;
        delete pPL3;
    }

    return iResult;
}

