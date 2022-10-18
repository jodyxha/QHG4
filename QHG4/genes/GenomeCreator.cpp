/*============================================================================
| GenomeCreator
| 
|  Creates a number of initial genome sequences according to various schemes:
|  - completely random               ("random")
|  - variants of a random            ("variants:<NumMutations>")
|  - all genes 0                     ("zero")
|  - hardy-wrinberg distribution     ("hw:<siterate>:<mutrate>")
|
|  GenomeCreator's template is usuallay GeneUtils or BitGeneUtils
|  
|  Author: Jody Weissmann
\===========================================================================*/ 

#include <cstdio>
#include <cstring>
#include <omp.h>

#include "strutils.h"
#include "stdstrutilsT.h"
#include "GenomeCreator.h"

#define INIT_STATE_NONE     0
#define INIT_STATE_VARIANTS 1
#define INIT_STATE_RANDOM   2
#define INIT_STATE_ZERO     3
#define INIT_STATE_HW       4


#define INIT_NAME_NONE     "none"
#define INIT_NAME_VARIANTS "variants"
#define INIT_NAME_RANDOM   "random"
#define INIT_NAME_ZERO     "zero"
#define INIT_NAME_HW       "hw"

//-----------------------------------------------------------------------------
// constructor
//
template<class U>
GenomeCreator<U>::GenomeCreator(uint iNumParents)
    : m_iNumParents(iNumParents),
      m_iInitialMutations(0),
      m_dInitialMutRate(0),
      m_dInitialSiteRate(0),
      m_iInitialType(INIT_STATE_NONE),
      m_sInitString("") {
    
}


//-----------------------------------------------------------------------------
// determineInitData
//  parse the attrbut string for the initial genome.
//  Possible forms:
//    variants:<NumMutations>
//    random
//    zero
//    hw:<MutationRate>:<SiteFraction>
//  For a while, we will still "undestand" the old numeric codes
//
template<class U>
int GenomeCreator<U>::determineInitData(std::string sLine) {
    int iResult = 0;
    int iIM = 0;
    // legacy - remove numeric soon
    if (strToNum(sLine, &iIM)) {
        if ((iIM > 0) || (iIM == -2)) {
            m_iInitialType      = INIT_STATE_VARIANTS;
            m_iInitialMutations = iIM;
            m_dInitialMutRate   = 0;
            m_dInitialSiteRate  = 0;
        } else {
            switch (iIM) {
            case -1:
                m_iInitialType      = INIT_STATE_RANDOM;
                m_iInitialMutations = 0;
                m_dInitialMutRate   = 0;
                m_dInitialSiteRate  = 0;
                break;
            case -3:
                m_iInitialType      = INIT_STATE_ZERO;
                m_iInitialMutations = 0;
                m_dInitialMutRate   = 0;
                m_dInitialSiteRate  = 0;
                break;
            case -4:
                m_iInitialType      = INIT_STATE_HW;
                m_iInitialMutations = 0;
                m_dInitialMutRate   = 0.05;
                m_dInitialSiteRate  = 0.1;
                break;
            default:
                iResult = -1;
            }
        }
    } else {
        // split
        iResult = -1;
        sLine = sLine.substr(sLine.find("="));
        stringvec vParts;
        uint iNum = splitString(sLine, vParts, ":");

        if (iNum > 0) {
	
            if (vParts[0] == INIT_NAME_VARIANTS) {
                int iN = 0;
                if (strToNum(vParts[1], &iN)) {
                    m_iInitialType      = INIT_STATE_VARIANTS;
                    m_iInitialMutations = iN;
                    m_dInitialMutRate   = 0;
                    m_dInitialSiteRate  = 0;
                    iResult = 0;
                } else {
                    stdprintf("expected [%s:<num_mutations>]\n", INIT_NAME_VARIANTS);
                }
            } else if (vParts[0] == INIT_NAME_RANDOM) {
                m_iInitialType      = INIT_STATE_RANDOM;
                m_iInitialMutations = 0;
                m_dInitialMutRate   = 0;
                m_dInitialSiteRate  = 0;
                iResult = 0;
         

            } else if  (vParts[0] == INIT_NAME_ZERO) {
                m_iInitialType      = INIT_STATE_ZERO;
                m_iInitialMutations = 0;
                m_dInitialMutRate   = 0;
                m_dInitialSiteRate  = 0;
                iResult = 0;

            } else if  (vParts[0] == INIT_NAME_HW) {
                double dR = 0;
                double dS = 0;
                const std::string &s1 = vParts[1];
                const std::string &s2 = vParts[2];
                if ((!s1.empty()) && (!s2.empty())) {
                    if (strToNum(s1, &dR)) {
                        if (strToNum(s2, &dS)) {
                            m_iInitialType      = INIT_STATE_HW;
                            m_iInitialMutations = 0;
                            m_dInitialMutRate   = dR;
                            m_dInitialSiteRate  = dS;
                            iResult = 0;
                        } else {
                            stdprintf("Not a number: [%s]\n", s2);
                        }
                    } else {
                        stdprintf("Not a number: [%s]\n", s1);
                    }
                } else {
                    stdprintf("expected [%s:<mut_rate>:<site_rate>]\n", INIT_NAME_HW);
                }
            }
        } else {
            stdprintf("empty params?\n");
        }
    }
    if (iResult == 0) {
        buildInitString();
        //        printf("Successfully parsed and rebuilt initial genome info:[%s]\n", m_sInitString.c_str());
    }
    return iResult;
}


//-----------------------------------------------------------------------------
// buildInitString
//   build a string for output in a QDF file
//
template<class U>
void GenomeCreator<U>::buildInitString() {

    switch (m_iInitialType) {
    case INIT_STATE_NONE:
        m_sInitString = INIT_NAME_NONE;
        break;
    
    case INIT_STATE_VARIANTS:
        m_sInitString = stdsprintf("%s:%d", INIT_NAME_VARIANTS, m_iInitialMutations);
        break;
    
    case INIT_STATE_RANDOM:
        m_sInitString = INIT_NAME_RANDOM;
        break;
    
    case INIT_STATE_ZERO:
        m_sInitString = INIT_NAME_ZERO;
        break;
    
    case INIT_STATE_HW:
        m_sInitString = stdsprintf("%s:%4.2e:%4.2e", INIT_NAME_HW, m_dInitialMutRate, m_dInitialSiteRate);
        break;

    default:
        m_sInitString = INIT_NAME_NONE;
    }
}


//-----------------------------------------------------------------------------
// createInitialGenomes
//   only do this if the buffer has been already added to the controller
//
template<class U>
int GenomeCreator<U>::createInitialGenomes(int iGenomeSize, int iNumGenomes, LayerArrBuf<ulong> &aGenome, WELL512 **apWELL) {
    int iResult = 0;
    printf("initial type: %d\n", m_iInitialType);
            
    ulong *pG0;
    ulong *pGenome;
    int iNumBlocks  = U::numNucs2Blocks(iGenomeSize);

    switch (m_iInitialType) {
    case INIT_STATE_NONE:
        // nothing to do
        break;

    case INIT_STATE_VARIANTS: 
        if (m_iInitialMutations > 0) {
            printf("Creating %d variants with %d mutations\n", iNumGenomes, m_iInitialMutations);
            pG0 = U::createRandomAlleles(m_iNumParents, iGenomeSize, m_iInitialMutations, apWELL[omp_get_thread_num()]);
            for (int i = 0; i < iNumGenomes; i++) {
                // go to position for next genome
                pGenome = &(aGenome[i]); 
                U::copyAndMutateGenes(m_iNumParents, iGenomeSize, m_iInitialMutations, pG0, pGenome);
            }
            delete[] pG0;
        }
        break;

    case INIT_STATE_RANDOM: 
        printf("Creating %d totally random genes\n", iNumGenomes);
        for (int i = 0; i < iNumGenomes; i++) {
            pGenome = &(aGenome[i]); 
            pG0 = U::createFullRandomGenes(m_iNumParents, iGenomeSize,  apWELL[omp_get_thread_num()]);
            memcpy(pGenome, pG0, m_iNumParents*iNumBlocks*sizeof(ulong));
            delete[] pG0;
        }
        break;


    case INIT_STATE_ZERO: 
        printf("Creating %d flat zeroed genes\n", iNumGenomes);
        for (int i = 0; i < iNumGenomes; i++) {
            pGenome = &(aGenome[i]); 
            memset(pGenome, 0, m_iNumParents*iNumBlocks*sizeof(ulong));
        }
            
        break;

    case INIT_STATE_HW: 
        printf("Creating %d genes at Hardy-Weinberg equilibrium, site ratio %4.2e, mutation rate %4.2e\n", iNumGenomes, m_dInitialSiteRate, m_dInitialMutRate);
        if ((m_dInitialSiteRate > 0) && (m_dInitialMutRate > 0)) {
            for (int i = 0; i < iNumGenomes; i++) {
                ulong *pGenome1 = &(aGenome[i]); 
                memset(pGenome1, 0, m_iNumParents*iNumBlocks*sizeof(ulong));
            }
            std::vector<uint> vMutSites;
            ulong   iMask = (1<<U::BITSINNUC)-1;
            for (int i = 0; i < iGenomeSize; i++) {
                if (apWELL[omp_get_thread_num()]->wrandd() < m_dInitialSiteRate) {
                    uint iBlock = i/(U::NUCSINBLOCK);
                    uint iBit   = U::BITSINNUC*(i%U::NUCSINBLOCK);
                    for (uint j = 0; j < m_iNumParents*iNumGenomes; j++) {
                        if (apWELL[omp_get_thread_num()]->wrandd() < m_dInitialMutRate) {
                            int iGenome = j/m_iNumParents;
                            int iAllele = j%m_iNumParents;
                            ulong *pGenome2 = &(aGenome[iGenome]); 
                                    
                            pGenome2[iBlock+iAllele*iNumBlocks] |= iMask << iBit;
                        }
                    }
                }
            }
        } else {
        }
        break;
        
    default:
        printf("Bad initialType [%d]\n", m_iInitialType);
        iResult = -1;
                
    }

    return iResult;
}

