#include <cstdio>

#include "hdf5.h"
#include "QDFUtils.h"

#include "Agent2DataExtractor.h"


int main(int iArgC, char *apArgV[]) {
    int iResult = 0;

    if (iArgC > 1) {
        bool bList = true;
        std::string sFileName = apArgV[1];
        std::string sDSPath = "/Populations/sapiens/AgentDataSet";
        stringvec vItems;

        if (iArgC > 2) {
            for (int i = 2; i < iArgC;  i++) {
                vItems.push_back(apArgV[i]);
            }
            bList = false;
        }   
        
        Agent2DataExtractor *pADE = Agent2DataExtractor::createInstance(sFileName, sDSPath);
        if (pADE != NULL) {
            
            if (bList) {
                printf("Members of compund data type\n");
                pADE->listDataType();
            } else {
                printf("Extracting ");
                for (uint i = 0; i < vItems.size(); i++) {
                    printf(" [%s]", vItems[i].c_str());
                }
                printf("\n");

                struct_manager *pSM = pADE->extractVarV(vItems);
                if (pSM != NULL) {
                    int iNumItems = pADE->getNumItems();
                    val_manager1<double> *pVM = static_cast<val_manager1<double>*>(pSM->convert(iNumItems));
                    pSM->display(iNumItems);
                    printf("---\n");
                    pVM->display(iNumItems);
                
                    std::vector<std::pair<int, double>> vIV;
                    pSM->makeIndexedVals(iNumItems, vIV);
                    
                    printf("Got %zd values:n", vIV.size());
                    for (uint i = 0; i < vIV.size(); i++) {
                        printf("%d: %f\n", vIV[i].first, vIV[i].second);
                    }

                    delete pSM;
                    delete pVM;
                } else {
                    printf("Couldn't create struct manager\n");
                }
               
                
            }
            delete pADE;
        } else {
            printf("Couldn't create AgentDataExtractorn");
        }

    } else {
        printf("usage ; %s <qdf-file> [<item-name>]\n", apArgV[0]);
    }


    printf("TODO: can we do arbitrary number of vars with template recursion?\n");
    printf("TODO: maybe work with char arrays\n");

    return iResult;
}
