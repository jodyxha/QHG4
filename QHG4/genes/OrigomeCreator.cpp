/*============================================================================
| OrigomeCreator
| 
|  Creates a number of initial genome sequences according to various schemessequence of all 0 or all 1:
|
|  OrigomeCreator's template is usuallay GeneUtils or BitGeneUtils
|  
|  Author: Jody Weissmann
\===========================================================================*/ 

#include <cstdio>
#include <cstring>
#include <omp.h>
#include <string>

#include "strutils.h"
#include "stdstrutils.h"
#include "stdstrutilsT.h"
#include "OrigomeCreator.h"

#define INIT_STATE_NONE     0
#define INIT_STATE_CONST    1


#define INIT_NAME_NONE     "none"
#define INIT_NAME_CONST    "const"

//-----------------------------------------------------------------------------
// constructor
//
template<class U>
OrigomeCreator<U>::OrigomeCreator(uint iNumParents)
    : m_iNumParents(iNumParents),
      m_iInitialType(INIT_STATE_NONE),
      m_iOrigValue(0) {
    
}


//-----------------------------------------------------------------------------
// determineInitData
//  parse the attrbut string for the initial genome.
//  Possible forms:
//    const
//    random
//    zero
//    hw:<MutationRate>:<SiteFraction>
//  For a while, we will still "undestand" the old numeric codes
//
template<class U>
int OrigomeCreator<U>::determineInitData(std::string sLine) {
    int iResult = 0;

    if (!sLine.empty()) {
        size_t iPos1 = sLine.find("=");
        std::string sName = sLine.substr(0, iPos1);
        std::string sArgs = sLine.substr(iPos1+1);
        
        stringvec vParts;
        uint iNum = splitString(sArgs, vParts, ":");

        if (sName == INIT_NAME_CONST) {
            if (iNum == 2) {
                m_iInitialType      = INIT_STATE_CONST;
                m_iOrigValue = 0;
                if (vParts[1].front() != '0') {
                    m_iOrigValue = 1;
                }
                iResult = 0;
            } else {
                printf("expcted  [const]\n");
            }
        } else {
            stdprintf("Unknown Origome type [%s]\n", sName);
        }

        if (iResult == 0) {
            buildInitString();
            //        printf("Successfully parsed and rebuilt initial genome info:[%s]\n", m_sInitString.c str());
        }
    } else {
        printf("Empty Origome type\n");
    }
    return iResult;
}


//-----------------------------------------------------------------------------
// buildInitString
//   build a string for output in a QDF file
//
template<class U>
void OrigomeCreator<U>::buildInitString() {
    char *p1 = NULL;

    switch (m_iInitialType) {
    case INIT_STATE_NONE:
        m_sInitString = INIT_NAME_NONE;
        break;
    
    case INIT_STATE_CONST:
        m_sInitString = std::string(INIT_NAME_CONST)+":"+std::to_string(m_iOrigValue);
        break;
    
    default:
        m_sInitString = INIT_NAME_NONE;
    }

    if (p1 != NULL) {
        delete[] p1;
    }
}


//-----------------------------------------------------------------------------
// createInitialOrigomes
//   only do this if the buffer has been already added to the controller
//   
template<class U>
int OrigomeCreator<U>::createInitialOrigomes(int iGenomeSize, int iNumGenomes, LayerArrBuf<ulong> &aOrigome) {
    int iResult = 0;
    printf("initial type: %d\n", m_iInitialType);
            
    ulong *pG0;
    ulong *pOrigome;
    int iNumBlocks  = U::numNucs2Blocks(iGenomeSize);

    switch (m_iInitialType) {
    case INIT_STATE_NONE:
        // nothing to do
        break;

    case INIT_STATE_CONST: 
        printf("Creating %d const origomes of type %d\n", iNumGenomes, m_iOrigValue);
        for (int i = 0; i < iNumGenomes; i++) {
            pOrigome = &(aOrigome[i]); 

            //@@@@ here we have to fill it with const value  
            pG0 = U::fillConstValues(m_iNumParents, iGenomeSize,  m_iOrigValue);
            memcpy(pOrigome, pG0, m_iNumParents*iNumBlocks*sizeof(ulong));
            delete[] pG0;
        }
        break;


        
    default:
        printf("Bad initialType [%d]\n", m_iInitialType);
        iResult = -1;
                
    }

    return iResult;
}

