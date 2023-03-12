#include <cstdio>

#include "hdf5.h"
#include "QDFUtils.h"

#include "Agent3DataExtractor.h"


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
        
        Agent3DataExtractor *pADE = Agent3DataExtractor::createInstance(sFileName, sDSPath);
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
                
                    if (pVM != NULL) {
                        delete pVM;
                    }
                    delete pSM;
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


    printf("MUSMUSMSUST pas field names as stringvec\n");

    printf("TODO: sanity check at start: all names must differ\n");
    printf("TODO: pass field names as stringvec\n");
    printf("TODO: struct info as vector of structinfos\n");
    printf("TODO: can we do arbitrary number of vars with template recursion?\n");
    printf("TODO: maybe work with char arrays\n");

    
    return iResult;
}
