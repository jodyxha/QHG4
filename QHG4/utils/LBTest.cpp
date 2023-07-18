#include <cstdio>
#include <cstdlib>
#include <cmath>
#include <ctime>

#include "LayerBuf.h"
#include "LayerBuf.cpp"
#include "LBController.h"

//---------------------------------------------------------------------------
// dummy class to test memblocks
//
struct cucumber {
public:
    int m_i;
};

//---------------------------------------------------------------------------
// dummy class to test memblocks
//
struct tomato {
    int m_i;
    int m_j;
};


//---------------------------------------------------------------------------
// main1
//
int main1(int iArgC, char *apArgV[]) {

    int iLayerSize = 6;
    int iNum = 34;
    LayerBuf<cucumber> LC(iLayerSize, MB_DESTROY_DELAY+2);
    LayerBuf<tomato> LT(iLayerSize, MB_DESTROY_DELAY+2);
    //    LBController *pLBC = new LBController(iLayerSize);
    LBController *pLBC = new LBController();
    pLBC->init(iLayerSize);
    pLBC->addBuffer(&LC);
    pLBC->addBuffer(&LT);

    printf("After init: %u layers: %d elements\n", pLBC->getNumLayers(), pLBC->getNumUsed());

    for (int i = 0; i < iNum; i++) {
        long j = pLBC->getFreeIndex();
        LC[j].m_i = i+1;
        LT[j].m_i = 2*i;
        LT[j].m_j = i%3;
    }

    // show agents
    printf("After creation: %u layers: %d elements\n", pLBC->getNumLayers(), pLBC->getNumUsed());
    int iCur = pLBC->getFirstIndex(LBController::ACTIVE);
    while (iCur != LBController::NIL) {
        printf("%d: C[%d] T[%d,%d]\n", iCur, LC[iCur].m_i, LT[iCur].m_i, LT[iCur].m_j);
        iCur = pLBC->getNextIndex(LBController::ACTIVE, iCur);
    }

    // delete some agents
    iCur = pLBC->getFirstIndex(LBController::ACTIVE);
    while (iCur != LBController::NIL) {
        int iNext = pLBC->getNextIndex(LBController::ACTIVE, iCur);
        if ( ((iCur % 3) == 2) || ((iCur % 7) > 3)) {
            pLBC->deleteElement(iCur);
        }
        iCur = iNext;
    }

    // show agents again
    printf("After deletions: %u layers: %d elements\n", pLBC->getNumLayers(), pLBC->getNumUsed());
    iCur = pLBC->getFirstIndex(LBController::ACTIVE);
    while (iCur != LBController::NIL) {
        printf("%d: C[%d] T[%d,%d]\n", iCur, LC[iCur].m_i, LT[iCur].m_i, LT[iCur].m_j);
        iCur = pLBC->getNextIndex(LBController::ACTIVE, iCur);
    }
    
    // compact data
    pLBC->compactData();

    // show agents again
    printf("After compacting: %u layers: %d elements\n", pLBC->getNumLayers(), pLBC->getNumUsed());
    iCur = pLBC->getFirstIndex(LBController::ACTIVE);
    while (iCur != LBController::NIL) {
        printf("%d: C[%d] T[%d,%d]\n", iCur, LC[iCur].m_i, LT[iCur].m_i, LT[iCur].m_j);
        iCur = pLBC->getNextIndex(LBController::ACTIVE, iCur);
    }
    
    delete pLBC;

    return 0;
}

//---------------------------------------------------------------------------
// compactbigtest
//
int compactbigtest(int iLayerSize, int iNumAgents, float fDeletePercentage) {
    int iResult = 0;

    printf("Preparing array\n");
    LayerBuf<cucumber> LC(iLayerSize, MB_DESTROY_DELAY+2);
    LBController *pLBC = new LBController();
    pLBC->init(iLayerSize);
    pLBC->addBuffer(&LC);

    clock_t ta = clock();
    for (int i = 0; i < iNumAgents; i++) {
        pLBC->getFreeIndex();   
    }

    int iCur = pLBC->getFirstIndex(LBController::ACTIVE);
    while (iCur != LBController::NIL) {
        
        int iNext = pLBC->getNextIndex(LBController::ACTIVE,iCur);    

        float r = (1.0*rand())/RAND_MAX;
        if (r < fDeletePercentage) {
            pLBC->deleteElement(iCur);   
        }

        iCur = iNext;
    }
    clock_t tb= clock();
    printf("creation: %ld clocks\n", tb -ta);    
    printf("have %d elements in %d layers\n", pLBC->getNumUsed(), pLBC->getNumLayers());
    clock_t t1 = clock();
    pLBC->compactData();
    clock_t t2 = clock();
    printf("compact %ld clocks\n", t2-t1);
    printf("have %d elements in %d layers\n", pLBC->getNumUsed(), pLBC->getNumLayers());
    
    delete pLBC;
    return iResult;
}

//---------------------------------------------------------------------------
// main2
//
int main3(int iArgC, char *apArgV[]) {
    int iResult = 0;
    //    iResult = main1(iArgC, apArgV);

    if (iArgC > 3) {
        int iNumLayers = atoi(apArgV[1]);
        int iNumAgents = atoi(apArgV[2]);
        float fPercentage = atof(apArgV[3]);
        iResult = compactbigtest(iNumLayers, iNumAgents, fPercentage);
    } else {
        printf("%s <numlayers> <numagents> <dletePercentage>\n", apArgV[0]);
    }
    return iResult;
}

//---------------------------------------------------------------------------
// main
//
int main(int iArgC, char *apArgV[]) {
    int iResult = 0;

    int iLayerSize = 7;
    /*
    printf("Preparing array\n");
    LayerBuf<cucumber> LC(iLayerSize, MB_DESTROY_DELAY+2);
    LBController *pLBC = new LBController();
    pLBC->init(iLayerSize);
    pLBC->addBuffer(&LC);

    
    for (int i = 0; i < 6; i++) {
        pLBC->getFreeIndex();   
    }
    for (int i = 0; i < 5; i++) {
        pLBC->getFreeIndex();   
    }
    pLBC->deleteElement(0);
    pLBC->deleteElement(1);
    pLBC->deleteElement(5);
    
    //    pLBC->deleteElement(8);
    printf("L: %d, F%d, L%d\n", pLBC->getNumLayers(), pLBC->getFirstIndex(LBController::ACTIVE), pLBC->getLastIndex(LBController::ACTIVE));
    for (uint i = 0; i < pLBC->getNumLayers(); i++) {
        pLBC->displayArray(i, 0, LBController::NIL);
    }

    pLBC->reserveSpace2(7);
    printf("reserveSpace(7)\n");
    printf("L: %d, F%d, L%d\n", pLBC->getNumLayers(), pLBC->getFirstIndex(LBController::ACTIVE), pLBC->getLastIndex(LBController::ACTIVE));
    for (uint i = 0; i < pLBC->getNumLayers(); i++) {
        pLBC->displayArray(i, 0, LBController::NIL);
    }

    pLBC->compactData();

    printf("compactData()\n");
    printf("L: %d, F%d, L%d\n", pLBC->getNumLayers(), pLBC->getFirstIndex(LBController::ACTIVE), pLBC->getLastIndex(LBController::ACTIVE));
    for (uint i = 0; i < pLBC->getNumLayers(); i++) {
        pLBC->displayArray(i, 0, LBController::NIL);
    }

    for (int i = 0; i < 8; i++) {   
        pLBC->deleteElement(i+2);
    }

    printf("8 deletes\n");
    printf("L: %d, F%d, L%d\n", pLBC->getNumLayers(), pLBC->getFirstIndex(LBController::ACTIVE), pLBC->getLastIndex(LBController::ACTIVE));
    for (uint i = 0; i < pLBC->getNumLayers(); i++) {
        pLBC->displayArray(i, 0, LBController::NIL);
    }

    pLBC->compactData();

    printf("compactData()\n");
    printf("L: %d, F%d, L%d\n", pLBC->getNumLayers(), pLBC->getFirstIndex(LBController::ACTIVE), pLBC->getLastIndex(LBController::ACTIVE));
    for (uint i = 0; i < pLBC->getNumLayers(); i++) {
        pLBC->displayArray(i, 0, LBController::NIL);
    }
    

    delete pLBC;
 
    */

    LayerBuf<cucumber> LC1(iLayerSize, MB_DESTROY_DELAY+2);
    
    LBController *pLBC1 = new LBController();
    pLBC1->init(iLayerSize);
    pLBC1->addBuffer(&LC1);
    
    LayerBuf<cucumber> LC2(iLayerSize, MB_DESTROY_DELAY+2);
    LBController *pLBC2 = new LBController();
    pLBC2->init(iLayerSize);
    pLBC2->addBuffer(&LC2);
    
    

    for (int i = 0; i < 6; i++) {
        pLBC1->getFreeIndex();   
    }
    for (int i = 0; i < 5; i++) {
        pLBC1->getFreeIndex();   
    }
    
    pLBC1->deleteElement(0);
    pLBC1->deleteElement(1);
    pLBC1->deleteElement(5);
    
    pLBC1->compactData();
    
    printf("*** LBC1 L: %d, F%d, L%d\n", pLBC1->getNumLayers(), pLBC1->getFirstIndex(LBController::ACTIVE), pLBC1->getLastIndex(LBController::ACTIVE));
    for (uint i = 0; i < pLBC1->getNumLayers(); i++) {
        printf("***   ");
        pLBC1->displayArray(i, 0, LBController::NIL);
    }
    
    
    for (int i = 0; i < 8; i++) {
        pLBC2->getFreeIndex();   
    }
    for (int i = 0; i < 8; i++) {
        pLBC2->getFreeIndex();   
    }
    pLBC2->deleteElement(3);
    pLBC2->deleteElement(2);
    pLBC2->deleteElement(0);
    
    pLBC2->compactData();
    
    printf("*** LBC2 L: %d, F%d, L%d\n", pLBC2->getNumLayers(), pLBC2->getFirstIndex(LBController::ACTIVE), pLBC2->getLastIndex(LBController::ACTIVE));
    for (uint i = 0; i < pLBC2->getNumLayers(); i++) {
        printf("***   ");
        pLBC2->displayArray(i, 0, LBController::NIL);
    }
    
    
    pLBC1->appendLBC(pLBC2);
    
    /*
    printf("LBC1combined  L: %d, F%d, L%d\n", pLBC1->getNumLayers(), pLBC1->getFirstIndex(LBController::ACTIVE), pLBC1->getLastIndex(LBController::ACTIVE));
    for (uint i = 0; i < pLBC1->getNumLayers(); i++) {
        pLBC1->displayArray(i, 0, LBController::NIL);
    }
    */

    pLBC1->compactData();

    
    printf("*** LBC1 after compact  L: %d, F%d, L%d\n", pLBC1->getNumLayers(), pLBC1->getFirstIndex(LBController::ACTIVE), pLBC1->getLastIndex(LBController::ACTIVE));
    for (uint i = 0; i < pLBC1->getNumLayers(); i++) {
        printf("***   ");
        pLBC1->displayArray(i, 0, LBController::NIL);
    }
    
    printf("*** LBC2 after compact  L: %d, F%d, L%d\n", pLBC2->getNumLayers(), pLBC2->getFirstIndex(LBController::ACTIVE), pLBC2->getLastIndex(LBController::ACTIVE));
    for (uint i = 0; i < pLBC2->getNumLayers(); i++) {
        printf("***   ");
        pLBC2->displayArray(i, 0, LBController::NIL);
    }
    
    delete pLBC2;

    delete pLBC1;
    
    return iResult;
}
