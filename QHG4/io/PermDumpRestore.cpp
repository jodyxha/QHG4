#include <cstdio>
#include <cstring>
#include <string>

#include <hdf5.h>
#include "Permutator.h"
#include "QDFUtils.h"
#include "QDFUtilsT.h"
#include "PermDumpRestore.h"

const static std::string PERM_ATTR_NUM_STATES = "PermNumStates";
const static std::string PERM_ATTR_CUR_SIZE   = "PermCurSize";
const static std::string PERM_ATTR_PREV_SIZE  = "PermPrevSize";
const static std::string PERM_ATTR_BUFFERS    = "PermBuffers";

//-----------------------------------------------------------------------------
//  dumpPerm
//    dumps permutators to QDF
//
int dumpPerm(Permutator **apPerm, int iNumPerm, const std::string sOwner, hid_t hSpeciesGroup) {

    int iResult = 0;

    char sAttrName[1024];
    uint iTotSize = 0;

    // number of Perm states
    if (iResult == 0) {
        sprintf(sAttrName, "%s_%s", sOwner.c_str(), PERM_ATTR_NUM_STATES.c_str());
        iResult = qdf_insertAttribute(hSpeciesGroup, sAttrName, 1, &iNumPerm); 
    }

    // current size of the perm buffer
    uint32_t *auiCurSizes = new uint32_t[iNumPerm];
    if (iResult == 0) {
        sprintf(sAttrName, "%s_%s", sOwner.c_str(), PERM_ATTR_CUR_SIZE.c_str());
        for (int i = 0; i < iNumPerm; i++) {
            auiCurSizes[i] = apPerm[i]->getSize();
            iTotSize += apPerm[i]->getSize();
        }
        iResult = qdf_insertAttribute(hSpeciesGroup, sAttrName, iNumPerm, auiCurSizes); 
    }
    delete[] auiCurSizes;

    // previous size of the perm buffer

    uint32_t *auiPrevSizes = new uint32_t[iNumPerm];
    if (iResult == 0) {
        sprintf(sAttrName, "%s_%s", sOwner.c_str(), PERM_ATTR_PREV_SIZE.c_str());
        for (int i = 0; i < iNumPerm; i++) {
            auiPrevSizes[i] = apPerm[i]->getPrevTot();
        }
        iResult = qdf_insertAttribute(hSpeciesGroup, sAttrName, iNumPerm, auiPrevSizes); 
    }
    delete[] auiPrevSizes;


    // the PERM states
    if (iResult == 0) {
        sprintf(sAttrName, "%s_%s", sOwner.c_str(), PERM_ATTR_BUFFERS.c_str());
            
        uint32_t *pSuperState = new uint32_t[iTotSize];
        uint32_t *pCur = pSuperState;
        for (int i = 0; i < iNumPerm; i++) {
            memcpy(pCur, apPerm[i]->getPerm(), apPerm[i]->getSize()*sizeof(uint32_t));
            pCur +=   apPerm[i]->getSize();
        }
        iResult = qdf_insertAttribute(hSpeciesGroup, sAttrName, iTotSize, pSuperState); 
        delete[] pSuperState;
    }

    return iResult;
}

//-----------------------------------------------------------------------------
//  restorePerm
//    restores Permutators from QDF
//    
//
int restorePerm(Permutator **apPerm, int iNumPerm, const std::string sOwner, hid_t hSpeciesGroup) {
    int iResult = 0;

    int iNumStates = 0;
    char sAttrName[1024];
        

    // number of Permutator states dumped
    sprintf(sAttrName, "%s_%s", sOwner.c_str(), PERM_ATTR_NUM_STATES.c_str());
    printf("[restorePerm] searching for attribute [%s]\n", sAttrName);fflush(stdout);
    iResult = qdf_extractAttribute(hSpeciesGroup, sAttrName, 1, &iNumStates); 
    if (iNumStates == iNumPerm) {

        uint iTotSize = 0;
        uint32_t *auiCurSizes = new uint32_t[iNumStates];
    
        if (iResult == 0) {
            sprintf(sAttrName, "%s_%s", sOwner.c_str(), PERM_ATTR_CUR_SIZE.c_str());
            printf("[restorePerm] searching for attribute [%s]\n", sAttrName);fflush(stdout);
            // current index values of the states
            iResult = qdf_extractAttribute(hSpeciesGroup, sAttrName, iNumStates, auiCurSizes); 
        }            
        for (int i = 0; i < iNumStates; i++) {
            iTotSize += auiCurSizes[i];
        }

        uint32_t *auiPrevSizes = new uint32_t[iNumStates];
    
        if (iResult == 0) {
            sprintf(sAttrName, "%s_%s", sOwner.c_str(), PERM_ATTR_PREV_SIZE.c_str());
            printf("[restorePerm] searching for attribute [%s]\n", sAttrName);fflush(stdout);
            // current index values of the states
            iResult = qdf_extractAttribute(hSpeciesGroup, sAttrName, iNumStates, auiPrevSizes); 
        }            

        if (iResult == 0) {
            // The Perm states themselves
            sprintf(sAttrName, "%s_%s", sOwner.c_str(), PERM_ATTR_BUFFERS.c_str());
            printf("[restorePerm] searching for attribute [%s]\n", sAttrName);fflush(stdout);
            
            uint32_t *pSuperState = new uint32_t[iTotSize];
            uint32_t *pCur = pSuperState;
            iResult = qdf_extractAttribute(hSpeciesGroup, sAttrName, iTotSize, pSuperState); 
            if (iResult == 0) {

                int i = 0;
                while (i < iNumStates) {
                    apPerm[i]->setSize(auiCurSizes[i]);
                    apPerm[i]->setPrevTot(auiPrevSizes[i]);
                    apPerm[i]->setPerm(pCur);
                    pCur +=  auiPrevSizes[i],
                    i++;
                }
                // if there are more threads than saved states, the Permutators keep their states from the constructor
                // it doesn't matter, because continuing with a different number of threads changes everxthing
            }

            delete[] pSuperState;
            delete[] auiPrevSizes;
            delete[] auiCurSizes;
        }

    } else {
        printf("Number of dumped Permutators differs from expected number (%d != %d)\n", iNumStates, iNumPerm);
        iResult = -1;
    }
    return iResult;
}

