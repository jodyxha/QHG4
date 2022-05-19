#include <cstdio>
#include <omp.h>

#include "types.h"
#include "utils.h"

#include "OccHistory.h"
#include "OccAnalyzer.h"

//----------------------------------------------------------------------------
//  constructor
//
OccAnalyzer::OccAnalyzer(OccHistory *pOH, const stringvec &vPopNames)
    :m_pOH(pOH),
     m_vPopNames(vPopNames) {
}

//----------------------------------------------------------------------------
//  destructor
//
OccAnalyzer::~OccAnalyzer() {
}


//----------------------------------------------------------------------------
//  showBitOrder
//
void OccAnalyzer::showBitOrder() {
    printf("BitOrder: ");
    for (uint i = 0; i < m_vPopNames.size(); i++) {
        if (i > 0) {
            printf(", ");
        }
        printf("%s", m_vPopNames[m_vPopNames.size()-1-i].c_str());
    }
    printf("\n");
}


//----------------------------------------------------------------------------
//  showHistory
//
void OccAnalyzer::showHistory(const timed_bits &tb) {

    timed_bits::const_iterator it;
    //    for (uint i = 0; i < tb.size(); i++) {
    for (it = tb.begin(); it != tb.end(); ++it) {
        uint m = it->second;
        //    uint m = tb[i].second;
        int iN = m_vPopNames.size()-1;
        char sT[64];
        if (it->first == int(NEG_INF)) {
            strcpy(sT, "-inf     ");
        } else {
            sprintf(sT, "%08d  ", it->first);
        }
        printf("%10s (%04x); ", sT, m);
        //        printf("T%6.1f (%08x); ", tb[i].first, m);
        while (iN >= 0) {
            printf("%d ", (m & 0x1));
            iN--;
            m >>= 1;
        }
        printf("\n");
    }
}


//----------------------------------------------------------------------------
//  showHistory
//
void OccAnalyzer::showHistory(int iCellID) {
    int iCellIndex = m_pOH->ID2Idx(iCellID);
    if (iCellIndex >= 0) {
        const timed_bits *pTB = m_pOH->getData();
        const timed_bits &tb = pTB[iCellIndex];
        showHistory(tb);
    } else {
        printf("[showHistory] Unknown cell ID [%d]\n", iCellID);
    }
}


//----------------------------------------------------------------------------
//  occupationAtTimeIndex
//
uint OccAnalyzer::occupationAtTimeIndex(float fT, int iCellIndex) {
    
    // find entry preceding fT
    const timed_bits &tb = m_pOH->getData()[iCellIndex];
   
    uint prevM  = 0;
    timed_bits::const_iterator it = tb.begin(); 
    while ((it != tb.end()) && (!isfinite(it->first) || (it->first < fT))) {
        prevM = it->second;
        it++;
    }
    return  prevM;
}


//----------------------------------------------------------------------------
//  occupationAtTime
//
uint OccAnalyzer::occupationAtTime(float fT, int iCellID) {
    
    int iCellIndex = m_pOH->ID2Idx(iCellID);
    return  occupationAtTimeIndex(fT, iCellIndex);
}


//----------------------------------------------------------------------------
//  occupationAtTime
//
uint *OccAnalyzer::occupationAtTime(float fT, uint *pVals) {
    // occupationAtTime(float fT, int iCellID) expects cell IDs
    const intvec &vCellIDs = m_pOH->getCellIDs();
    for (uint i = 0; i < vCellIDs.size(); i++) {
        // 
        pVals[i] = occupationAtTime(fT, vCellIDs[i]);
    }

    return pVals;
}


//----------------------------------------------------------------------------
//  occupationOfRegion
//
int OccAnalyzer::occupationOfRegion(intvec &vCellIDs, timed_bits &tb) {
    int iResult = 0;
   
    // we first collect all event times
    std::set<float> sTimes;
    for (uint i = 0; (iResult == 0) && (i < vCellIDs.size()); i++) {
        int iCellIndex = m_pOH->ID2Idx(vCellIDs[i]);
        if (iCellIndex >= 0) {
            
            const timed_bits &tt=m_pOH->getData()[iCellIndex];
            timed_bits::const_iterator itt;
            for (itt = tt.begin(); itt != tt.end(); ++itt) {
                sTimes.insert(itt->first);
            }
        } else {
            printf("Cell with ID [%d] not found in originals\n", vCellIDs[i]);
            iResult = -1;
        }
    }
    std::set<float>::const_iterator its;
    /*   
    // display times
    for (its = sTimes.begin(); its != sTimes.end(); ++its) {
        printf("  %6.1f", *its);
    }
    printf("\n");
    */
    // now for each event time either we have an event or else we have to interpolate
    // we have to get the last event's value
    for (its = sTimes.begin(); its != sTimes.end(); ++its) {
        //printf("at time  %6.1f adding:  ", *its);
        uint uMask = 0;
        for (uint i = 0; (iResult == 0) && (i < vCellIDs.size()); i++) {
            int iCellIndex = m_pOH->ID2Idx(vCellIDs[i]);
            if (iCellIndex >= 0) {
                //printf("c%d", iCells);
                const timed_bits &tt=m_pOH->getData()[iCellIndex];
                timed_bits::const_iterator itt  = tt.find(*its);
                
                uint uNew = 0;
                if (itt != tt.end()) {
                // we have event for this time, use it
                    uNew = itt->second;
                //printf("o");
                } else {
                // no event for this time,  et previous one
                    uNew = occupationAtTime(*its, i);
                    //printf("i");
                }
                uMask |= uNew;
                //printf("%04x  ", uNew);
            } else {
                printf("unknown ID [%d[]\n", vCellIDs[i]);
                iResult = -1;
            }
        }
        tb[*its] = uMask;
        //printf(" -> %04x\n", uMask);
    }

    return iResult;
}


//----------------------------------------------------------------------------
//  translateIDs
//
int OccAnalyzer::translateIDs(intvec &vCellIDs, intvec &vCellIndexes) {
    int iResult = 0;
#pragma omp parallel for reduction(+:iResult)
    for (uint i = 0; i < vCellIDs.size(); i++) {
        int iCellIndex = m_pOH->ID2Idx(vCellIDs[i]);
        if (iCellIndex >= 0) {
            vCellIndexes.push_back(iCellIndex);
        } else {
            iResult = -1;
        }
    }
    return iResult;
}
    

//----------------------------------------------------------------------------
//  removeZeroStates
//
timed_bits *OccAnalyzer::removeZeroStates() {
    const intvec &vCellIDs = m_pOH->getCellIDs(); 
    timed_bits *pTBRed = new timed_bits[vCellIDs.size()]; 
    
#pragma omp parallel for
    for (uint i = 0; i < vCellIDs.size(); i++) {
        // this should be ok, because we assume that OccHistory's IDs are known
        int iCellIndex = m_pOH->ID2Idx(vCellIDs[i]);
        const timed_bits &tt=m_pOH->getData()[iCellIndex];
        timed_bits ttRed;
            
        uint uMaskPrev = 0xffffffff;
        timed_bits::const_iterator it;
        for (it =  tt.begin(); it != tt.end(); ++it) {
            if ((it->second != uMaskPrev) && (it->second != 0)) {
                uMaskPrev = it->second;
                ttRed[it->first] = it->second;
            }
        }
        pTBRed[i] = ttRed;
    }
    return pTBRed;
}


//----------------------------------------------------------------------------
//  mergeRegionStates
//
timed_bits *OccAnalyzer::mergeRegionStates(intvec &vRegionIDs, bool bIgnoreZeros) {
    timed_bits *ptt = NULL;
    int iResult = 0;
    // verify IDs
    intvec vIndexes;
    iResult = translateIDs(vRegionIDs, vIndexes);

    if (iResult == 0) {
        // collect time points
        intset *asTimes = new intset[omp_get_max_threads()];
        // to parallel this we need an array of sTimes and merge them later
#pragma omp parallel for
        for (uint iCellIndex = 0; iCellIndex < vIndexes.size(); iCellIndex++) {
            int iT = omp_get_thread_num();
            const timed_bits &tt=m_pOH->getData()[iCellIndex];
            timed_bits::const_iterator it;
            for (it =  tt.begin(); it != tt.end(); ++it) {
                asTimes[iT].insert(it->first);
            }
        }
        // now merge
        for (int iT = 1; iT < omp_get_max_threads(); iT++) {
            asTimes[0].insert(asTimes[iT].begin(), asTimes[iT].end());
        }
        // cllect masks at timeepoints
        ptt = new timed_bits;
        intset::const_iterator its;
        uint uPrev = 0xffffffff;
        for (its = asTimes[0].begin(); its != asTimes[0].end(); its++) {
            uint uMask = 0;
            for (uint iCellIndex = 0; iCellIndex < vIndexes.size(); iCellIndex++) {
                uint uCur = occupationAtTimeIndex(*its, iCellIndex);
                uMask |= uCur;
                
            }
            if ((!bIgnoreZeros) || ((uMask != uPrev) && (uMask != 0))) {
                (*ptt)[*its] = uMask;
            }
        }
        delete[] asTimes;
    }
    return ptt;
}
