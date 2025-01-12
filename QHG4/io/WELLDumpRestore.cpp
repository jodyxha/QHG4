#include <cstdio>
#include <cstring>
#include <string>

#include <hdf5.h>
#include "WELL512.h"
#include "xha_strutilsT.h"
#include "QDFUtils.h"
#include "QDFUtilsT.h"
#include "WELLDumpRestore.h"

#define WELL_ATTR_NUM_STATES   "WELLNumStates"
#define WELL_ATTR_CUR_INDEX    "WELLCurIndex"
#define WELL_ATTR_FINAL_WELL   "WELLFinalState"
#define WELL_ATTR_LAST_NORMAL  "WELLLastNormal"
#define WELL_ATTR_LAST_SIGMA   "WELLLastSigma"

//-----------------------------------------------------------------------------
//  dumpWELL
//    dumps WELL to QDF
//
int dumpWELL(WELL512 **apWELL, int iNumWELL, const std::string sOwner, hid_t hSpeciesGroup) {
    int iResult = 0;

    std::string sAttrName = "";

    // number of WELL states
    if (iResult == 0) {
        sAttrName = xha_sprintf("%s_%s", sOwner, WELL_ATTR_NUM_STATES);
        iResult = qdf_insertAttribute(hSpeciesGroup, sAttrName, 1, &iNumWELL); 
    }

    // current indexes of the states
    uint32_t *auiIndexes = new uint32_t[iNumWELL];
    if (iResult == 0) {
        sAttrName = xha_sprintf("%s_%s", sOwner, WELL_ATTR_CUR_INDEX);
        for (int i = 0; i < iNumWELL; i++) {
            auiIndexes[i] = apWELL[i]->getIndex();
        }
        iResult = qdf_insertAttribute(hSpeciesGroup, sAttrName, iNumWELL, auiIndexes); 
    }
    delete[] auiIndexes;

    // current indexes of the states
    double *adLastNormals = new double[iNumWELL];
    if (iResult == 0) {
        sAttrName = xha_sprintf("%s_%s", sOwner, WELL_ATTR_LAST_NORMAL);
        for (int i = 0; i < iNumWELL; i++) {
            adLastNormals[i] = apWELL[i]->getPrevNormal();
        }
        iResult = qdf_insertAttribute(hSpeciesGroup, sAttrName, iNumWELL, adLastNormals); 
    }
    delete[] adLastNormals;

    // current indexes of the states
    double *adSigmas = new double[iNumWELL];
    if (iResult == 0) {
        sAttrName = xha_sprintf("%s_%s", sOwner, WELL_ATTR_LAST_SIGMA);
        for (int i = 0; i < iNumWELL; i++) {
            adSigmas[i] = apWELL[i]->getSigma();
        }
        iResult = qdf_insertAttribute(hSpeciesGroup, sAttrName, iNumWELL, adSigmas); 
    }
    delete[] adSigmas;


    // the WELL states
    if (iResult == 0) {
        sAttrName = xha_sprintf("%s_%s", sOwner, WELL_ATTR_FINAL_WELL);
            
        uint32_t *pSuperState = new uint32_t[iNumWELL*STATE_SIZE];
        uint32_t *pCur = pSuperState;
        for (int i = 0; i < iNumWELL; i++) {
            memcpy(pCur, apWELL[i]->getState(), STATE_SIZE*sizeof(uint32_t));
            pCur +=  STATE_SIZE;
        }
        iResult = qdf_insertAttribute(hSpeciesGroup, sAttrName, iNumWELL*STATE_SIZE, pSuperState); 
        delete[] pSuperState;
    }

    return iResult;
}

//-----------------------------------------------------------------------------
//  restoreWELL
//    restores WELL from QDF
//    
//
int restoreWELL(WELL512 **apWELL, int iNumWELL, const std::string sOwner, hid_t hSpeciesGroup) {
    int iResult = 0;

    int iNumStates = 0;
    std::string sAttrName = "";
        

    // number of WELL states dumped
    sAttrName = xha_sprintf("%s_%s", sOwner, WELL_ATTR_NUM_STATES);
    xha_printf("[restoreWELL] searching for attribute [%s]\n", sAttrName);fflush(stdout);
    iResult = qdf_extractAttribute(hSpeciesGroup, sAttrName, 1, &iNumStates); 
    if (iNumStates == iNumWELL) {
        uint32_t *auiIndexes    = new uint32_t[iNumStates];
        double   *adLastNormals = new double[iNumStates];
        double   *adLastSigmas  = new double[iNumStates];
        if (iResult == 0) {
            sAttrName = xha_sprintf("%s_%s", sOwner, WELL_ATTR_CUR_INDEX);
            xha_printf("[restoreWELL] searching for attribute [%s]\n", sAttrName);fflush(stdout);
            // current index values of the states
            iResult = qdf_extractAttribute(hSpeciesGroup, sAttrName, iNumStates, auiIndexes); 
        }            

        if (iResult == 0) {
            sAttrName = xha_sprintf("%s_%s", sOwner, WELL_ATTR_LAST_NORMAL);
            xha_printf("[restoreWELL] searching for attribute [%s]\n", sAttrName);fflush(stdout);
            // current index values of the states
            iResult = qdf_extractAttribute(hSpeciesGroup, sAttrName, iNumStates, adLastNormals); 
        }            

        if (iResult == 0) {
            sAttrName = xha_sprintf("%s_%s", sOwner, WELL_ATTR_LAST_SIGMA);
            xha_printf("[restoreWELL] searching for attribute [%s]\n", sAttrName);fflush(stdout);
            // current index values of the states
            iResult = qdf_extractAttribute(hSpeciesGroup, sAttrName, iNumStates, adLastSigmas); 
        }            

        if (iResult == 0) {
            // The WELL states themselves
            sAttrName = xha_sprintf("%s_%s", sOwner, WELL_ATTR_FINAL_WELL);
            xha_printf("[restoreWELL] searching for attribute [%s]\n", sAttrName);fflush(stdout);
            
            uint32_t *pSuperState = new uint32_t[iNumStates*STATE_SIZE];
            uint32_t *pCur = pSuperState;
            iResult = qdf_extractAttribute(hSpeciesGroup, sAttrName, iNumStates*STATE_SIZE, pSuperState); 
            if (iResult == 0) {

                int i = 0;
                while (i < iNumStates) {
                    apWELL[i]->seed(pCur, auiIndexes[i]);
                    apWELL[i]->setGaussStates(adLastNormals[i], adLastSigmas[i]);
                    pCur +=  STATE_SIZE;
                    i++;
                }
                // if there are more threads than saved states, the WELLs keep their states from the constructor
                // it doesn't matter, because continuing with a different number of threads changes everxthing
            }

            delete[] pSuperState;
            delete[] adLastSigmas;
            delete[] adLastNormals;
            delete[] auiIndexes;
        }

    } else {
        xha_printf("Number of dumped WELLs differs from expected number (%d != %d)\n", iNumStates, iNumWELL);
        iResult = -1;
    }
    return iResult;
}

