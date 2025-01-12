#include <cstdio>
#include <cstring>

#include "strutils.h"
#include "xha_strutils.h"
#include "xha_strutilsT.h"
#include "ParamProvider2.h"

#define ATTR_HYBRIDS_NAME              "Hybrids"
#define ATTR_HYBRIDS_GENOME_SIZE       "Hybrids_genome_size"
#define ATTR_HYBRIDS_NUM_CROSSOVER     "Hybrids_num_crossover"
#define ATTR_HYBRIDS_MUTATION_RATE     "Hybrids_mutation_rate"
#define ATTR_HYBRIDS_INITIAL_MUTS      "Hybrids_initial_muts"
#define ATTR_HYBRIDS_INITIAL_ORIS      "Hybrids_initial_oris"
#define ATTR_HYBRIDS_CREATE_NEW_GENOME "Hybrids_create_new_genome"
#define ATTR_HYBRIDS_BITS_PER_NUC      "Hybrids_bits_per_nuc"

template <typename T>
int getParamVal(const stringmap *pParams, const string &sName, T *pVal) {
    int iResult = -1;
    stringmap::const_iterator it = pParams->find(sName);
    if (it != pParams->end()) {
        if (strToNum(it->second, pVal)) {
            iResult = 0;
        } else {
            xha_printf("Invalid Number format [%s]\n", it->second);
        }
    } else {
        xha_printf("unknown param name [%s]\n", sName);
    }
    return iResult;
}


//-----------------------------------------------------------------------------
// readKeyStr
//   gets the value from a line of form
//   <Key> <Sep> <Val>
template<typename T>
int getParamValArr(const stringmap *pParams, const std::string &sName, int iNum, T *pVal) {
    int iResult = -1;
    stringmap::const_iterator it = pParams->find(sName);
    if (it != pParams->end()) {

        int iC = 0;
        iResult = 0;
        stringvec vParts
        uint iNumParts = splitString(it->second, vParts, " \t"); 

        T tDummy;
        for (uint i = 0; (iResult = 0) && (i < iNumParts) && (iC < iNum); i++)) {
            if (strToNum(vParts[i], &tDummy)) {
                pVal[iC++] = tDummy;
            } else {
                xha_printf("Invalid number for \"%s\" [%s]\n", pName, vParts[i]);
                iResult = -1;
            }
        }
    } else {
        xha_printf("unknown param name [%s]\n", sName);
    }
        
    return iResult;
}


//-----------------------------------------------------------------------------
// readKeyStr
//   gets the value from a line of form
//   <Key> <Sep> <Val>
template<typename T>
int getParamKeyStr(const stringmap *pParams, const std::string &sName, std::string &sVal) {
    int iResult = -1;
    stringmap::const_iterator it = pParams->find(sName);
    if (it != pParams->end()) {
        sVal = it->second;
        iResult = 0;
        }
    }
    return iResult;
}


//----------------------------------------------------------------------------
// main
//
int main(int iArgC, char *apArgV[]) {

    int iResult = -1;
    if (iArgC > 1) {
        

        ParamProvider2 *pPP = ParamProvider2::createInstance(apArgV[1]);
        if (pPP != NULL) {
            iResult = 0;
            pPP->showTree();
            const std::vector<std::string> &vClassName = pPP->getClassNames();
            for (uint i = 0; i < vClassName.size(); ++i){
                xha_printf("class [%s]\n", vClassName[i]);
                
                iResult = pPP->selectClass(vClassName[i]);
                
                const stringmap &cAttr = pPP->getClassInfo()->cattr;
                
                stringmap::const_iterator it;
                for (it = cAttr.begin(); it != cAttr.end(); ++it) {
                    
                    xha_printf("  %s -> %s\n", it->first, it->second);
                }
                
                std::string sActionName("Hybrids");
                const stringmap *pHybPar = pPP->getParams(sActionName);
                if (pHybPar != NULL) {
                    xha_printf("  Action[%s]\n", sActionName);
                    for (it = pHybPar->begin(); it != pHybPar->end(); ++it) {
                        
                        xha_printf("    %s -> %s\n", it->first, it->second);
                    }
                } else {
                    xha_printf("No params for [%s]\n",sActionName);
                }
                
                int iGenomeSize;
                double dMutationRate;
                iResult = 0;
                iResult += getParamVal(pHybPar, ATTR_HYBRIDS_GENOME_SIZE, &iGenomeSize);
                iResult += getParamVal(pHybPar, ATTR_HYBRIDS_MUTATION_RATE, &dMutationRate);

                if (iResult == 0) {
                    printf("Retrieved params\n");
                    printf("  genome size %d\n", iGenomeSize);
                    printf("  mutation rate %f\n", dMutationRate);
                }
                /*
                strcpy(sActionName, "SingleEvaluator");
                const stringmap *pSE = pPP->getParams(sActionName);
                */
                
            }

            /*
            const stringmap *pa = pPP->getParams("OoANavHybRelPop", "Hybrids");

            stringmap::const_iterator  ita;
            for (ita = pa->begin(); ita != pa->end(); ++ita) {
                xha_printf("  %s => %s\n", ita->first, ita->second);
            }
            */
            /*            
            const classes &mc  = pQXR->getClasses();
            classes::const_iterator itc;
            for (itc = mc.begin(); itc != mc.end(); ++itc) {
                xha_printf("------ class\n");
                xha_printf("class '%s'\n", itc->first);
                const stringmap &cattr = itc->second.cattr;
                stringmap::const_iterator  ita;
                for (ita = cattr.begin(); ita != cattr.end(); ++ita) {
                    xha_printf("  %s: %s\n", ita->first, ita->second);
                }
                xha_printf("------ modules\n");
                const modules &mods = itc->second.mods;
                modules::const_iterator itm;
                for (itm = mods.begin(); itm != mods.end(); ++itm) {
                    xha_printf("  module '%s'\n", itm->first);
                    stringmap::const_iterator  ita2;
                    for (ita2 = itm->second.begin(); ita2 != itm->second.end(); ++ita2) {
                        xha_printf("    %s: %s\n", ita2->first, ita2->second);
                    }
                    
                }
                xha_printf("------ priorities\n");
                const stringmap &pattr = itc->second.prios;
                stringmap::const_iterator  ita3;
                for (ita3 = pattr.begin(); ita3 != pattr.end(); ++ita3) {
                    xha_printf("  %s: %s\n", ita3->first, ita3->second);
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
