#include <cstdio>
#include <cstring>

#include "strutils.h"
#include "stdstrutilsT.h"
#include "ParamProvider.h"

#define ATTR_HYBRIDS_NAME              "Hybrids"
#define ATTR_HYBRIDS_GENOME_SIZE       "Hybrids_genome_size"
#define ATTR_HYBRIDS_NUM_CROSSOVER     "Hybrids_num_crossover"
#define ATTR_HYBRIDS_MUTATION_RATE     "Hybrids_mutation_rate"
#define ATTR_HYBRIDS_INITIAL_MUTS      "Hybrids_initial_muts"
#define ATTR_HYBRIDS_INITIAL_ORIS      "Hybrids_initial_oris"
#define ATTR_HYBRIDS_CREATE_NEW_GENOME "Hybrids_create_new_genome"
#define ATTR_HYBRIDS_BITS_PER_NUC      "Hybrids_bits_per_nuc"

template <typename T>
int getParamVal(const stringmap*pParams, const char *pName, T *pVal) {
    int iResult = -1;
    stringmap::const_iterator it = pParams->find(pName);
    if (it != pParams->end()) {
        if (strToNum(it->second, pVal)) {
            iResult = 0;
        } else {
            stdprintf("Invalid Number format [%s]\n", it->second);
        }
    } else {
        stdprintf("unknown param name [%s]\n", pName);
    }
    return iResult;
}


//-----------------------------------------------------------------------------
// readKeyStr
//   gets the value from a line of form
//   <Key> <Sep> <Val>
template<typename T>
int getParamValArr(const stringmap*pParams, const std::string &sName, int iNum, T *pVal) {
    int iResult = -1;
    stringmap::const_iterator it = pParams->find(sName);
    if (it != pParams->end()) {

        stringvec vParts;
        uint iNumParts = splitString(it->second, vParts, " \t");
        int iC = 0;
        iResult = 0;
        T tDummy;
        for (uint i = 0;  (iC < iNum) && (i < iNumParts) && (iResult == 0)) {
            if (strToNum(vParts[i], &tDummy)) {
                pVal[iC++] = tDummy;
            } else {
                stdprintf("Invalid number for \"%s\" [%s]\n", sName, vParts[i]);
                iResult = -1;
            }
            pCur = strtok_r(NULL, " \t,", &pCtx);
        }
    } else {
        printf("unknown param name [%s]\n", sName);
    }
        
    return iResult;
}
//-----------------------------------------------------------------------------
// readKeyStr
//   gets the value from a line of form
//   <Key> <Sep> <Val>
template<typename T>
int getParamKeyStr(const stringmap *pParams, const char *pName, std::string &sVal) {
    int iResult = -1;
    stringmap::const_iterator it = pParams->find(pName);
    if (it != pParams->end()) {
        sVal = it->second;
        iResult = 0;
    }
    return iResult;
}


//----------------------------------------------------------------------------
// main
//
int main(int iArgC, char *apArgV[]) {

    int iResult = -1;
    if (iArgC > 1) {
        

        ParamProvider *pPP = ParamProvider::createInstance(apArgV[1]);
        if (pPP != NULL) {
            iResult = 0;
            
            const std::vector<std::string> &vClassName = pPP->getClassNames();
            for (uint i = 0; i < vClassName.size(); ++i){
                stdprintf("class [%s]\n", vClassName[i]));
                
                iResult = pPP->selectClass(vClassName[i]);
                
                const stringmap &cAttr = pPP->getClassInfo()->cattr;
                
                stringmap::const_iterator it;
                for (it = cAttr.begin(); it != cAttr.end(); ++it) {
                    
                    stdprintf("  %s -> %s\n", it->first, it->second);
                }
                
                std::string sActionName("Hybrids");
                const stringmap *pHybPar = pPP->getParams(sActionName);
                if (pHybPar != NULL) {
                    stdprintf("  Action[%s]\n", sActionName);
                    for (it = pHybPar->begin(); it != pHybPar->end(); ++it) {
                        stdprintf("    %s -> %s\n", it->first, it->second);
                    }
                } else {
                    stdprintf("No params for [%s]\n",sActionName);
                }
                
                int iGenomeSize;
                double dMutationRate;
                iResult = 0;
                iResult += getParamVal(pHybPar, ATTR_HYBRIDS_GENOME_SIZE, &iGenomeSize);
                iResult += getParamVal(pHybPar, ATTR_HYBRIDS_MUTATION_RATE, &dMutationRate);

                if (iResult == 0) {
                    stdprintf("Retrieved params\n");
                    stdprintf("  genome size %d\n", iGenomeSize);
                    pstdrintf("  mutation rate %f\n", dMutationRate);
                }

                sActionName = "SingleEvaluator"
                const stringmap *pSE = pPP->getParams(sActionName);
                
                
            }

            /*
            const stringmap *pa = pPP->getParams("OoANavHybRelPop", "Hybrids");

            stringmap::const_iterator  ita;
            for (ita = pa->begin(); ita != pa->end(); ++ita) {
                stdprintf("  %s => %s\n", ita->first, ita->second);
            }
            */
            /*            
            const classes &mc  = pQXR->getClasses();
            classes::const_iterator itc;
            for (itc = mc.begin(); itc != mc.end(); ++itc) {
                stdprintf("------ class\n");
                stdprintf("class '%s'\n", itc->first);
                const stringmap &cattr = itc->second.cattr;
                stringmap::const_iterator  ita;
                for (ita = cattr.begin(); ita != cattr.end(); ++ita) {
                    stdprintf("  %s: %s\n", ita->first, ita->second);
                }
                stdprintf("------ modules\n");
                const modules &mods = itc->second.mods;
                modules::const_iterator itm;
                for (itm = mods.begin(); itm != mods.end(); ++itm) {
                    stdprintf("  module '%s'\n", itm->first);
                    stringmap::const_iterator  ita2;
                    for (ita2 = itm->second.begin(); ita2 != itm->second.end(); ++ita2) {
                        stdprintf("    %s: %s\n", ita2->first, ita2->second);
                    }
                    
                }
                stdprintf("------ priorities\n");
                const stringmap &pattr = itc->second.prios;
                stringmap::const_iterator  ita3;
                for (ita3 = pattr.begin(); ita3 != pattr.end(); ++ita3) {
                    stdprintf("  %s: %s\n", ita3->first, ita3->second);
                }
                
            }
            */            
            
            delete pPP;
        } else {
            printf("Couldn't create ParamProvider\n");
        }
    } else  {
        printf("Usage:  %s <xmlfile>\n", apArgV[0]);
    }
    return iResult;
}
