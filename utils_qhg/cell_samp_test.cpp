#include <vector>
#include <string>
#include <map>

#include "strutils.h"
#include "LineReader.h"
#include "ParamReader.h"
#include "QDFUtils.h"
#include "AnalysisUtils.h"
#include "CellSampler.h"

//----------------------------------------------------------------------------
//  main
//
int main(int iArgC, char *apArgV[]) {
    int iResult = 0;
    
    ParamReader *pPR = new ParamReader();
    if (pPR != NULL) {
        char *sLocationsFile = NULL;
        char *sGeoQDF        = NULL;

        pPR->setVerbose(true);
        bool bOK = pPR->setOptions(2,
                                   "-g:S",      &sGeoQDF,
                                   "-l:S!",     &sLocationsFile);
        if (bOK) {
            iResult = pPR->getParams(iArgC, apArgV);
                
            if (iResult >= 0) {
                

                CellSampler *pCS = CellSampler::createInstance(sGeoQDF, sLocationsFile, 0);
                if (pCS != NULL) {
                    const loc_cells& mSelected = pCS->getSelected();  
                    printf("Have %d cells\n", pCS->getNumCells());
                    printf("Have %zd regions\n", mSelected.size());
                    loc_cells::const_iterator it;
                    for (it = mSelected.begin(); it != mSelected.end(); ++it) {
                        printf("%s:\n", it->first.c_str());
                        printf("  ");
                        const std::vector<int> v = it->second;
                        for (uint i = 0; i < v.size(); i++) {
                            printf("%d ", v[i]);
                        }
                        printf("\n");
                    }
                    delete pCS;
                } else {
                    iResult = -1;
                    fprintf(stderr, "Couldn't create CellSampler\n");
                }
            } else {
                //  usage(apArgV[0]);
            }
        } else {
            fprintf(stderr, "Couldn't set ParamReader options\n");
        }
        delete pPR;
    } else {
        fprintf(stderr, "Couldn't create ParamReader\n");
    }

    return iResult;
}
