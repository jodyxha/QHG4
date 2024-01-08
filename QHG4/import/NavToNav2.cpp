#include <cstdio>
#include <cstdlib>
#include <cstring>

#include <string>

#include "hdf5.h"

#include "types.h"
#include "GroupReader.h"

#include "SCellGrid.h"
#include "strutils.h"
#include "QDFUtils.h"

#include "Navigation.h"
#include "NavGroupReader.h"

#include "Navigation2.h"
#include "NavWriter2.h"

#include "NavToNav2Converter.h"

//----------------------------------------------------------------------------
// usage
//
void usage(std::string sApp) {
    printf("%s - create a Navigation2 file from a Navigation file\n", sApp.c_str());
    printf("usage;\n");
    printf("  %s <input-file> <output-file> [<num-cells>]\n",  sApp.c_str());
}

//----------------------------------------------------------------------------
// readParams
//
int readParams(int iArgC, char *apArgV[], uint *piNumCells, std::string &sFileIn, std::string &sFileOut) {
    int iResult = -1;

    if (iArgC > 1) {
        iResult = 0;
        sFileIn = apArgV[1];
        if (iArgC > 2) {
            int i = 2;
            if (iArgC > 3) {
                sFileOut = apArgV[2];
                i = 3;
            } else {
                sFileOut = sFileIn;
            }
            int iTemp = 0;
            if (strToNum(apArgV[i], &iTemp)) {
                *piNumCells = iTemp;
            } else {
                iResult = -1;
                printf("Couldn't convert [%s] to int\n", apArgV[2]);
            }

        } else {
            printf("OK\n");
        }
    }

    return iResult;
}


//----------------------------------------------------------------------------
// mainold
//
int mainold(int iArgC, char *apArgV[]) {
    int iResult = 0;

    uint iNumCells = 660492;
    std::string sFileIn  = "";
    std::string sFileOut = "";
    
    iResult = readParams(iArgC, apArgV, &iNumCells, sFileIn, sFileOut);
    if (iResult == 0) {
        printf("FileIn:   [%s]\n", sFileIn.c_str());
        printf("FileOut:  [%s]\n", sFileOut.c_str());
        printf("NumCells: %d\n", iNumCells);
    
        hid_t hFileIn = qdf_openFile(sFileIn, true);
	if (hFileIn != H5P_DEFAULT) {

            stringmap sm;
            sm["DUMMY"] = "dummy";
            SCellGrid *pCG = new SCellGrid(0, iNumCells, sm);

            NavGroupReader *pNR = NavGroupReader::createNavGroupReader(hFileIn);
            Navigation *pNav = new Navigation(pCG);

            NavAttributes navatt;
    	    memset(&navatt, 0, sizeof(NavAttributes));
            iResult = pNR->readAttributes(&navatt);
            if (iResult == 0) {

                iResult = pNR->readData(pNav);
                if (iResult == 0) {
                    if (navatt.m_iNumBridges > 0) {
                        iResult = pNR->readBridges(pNav);
                    }
                    if (iResult == 0) {
                        pCG->setNavigation(pNav);
                    } else {
                        printf("Couldn't read bridges\n");
                    }
                } else {
                    printf("Couldn't read data\n");
                } 
            } else {
                printf(" Couldn't read attributes\n");
            }

	    qdf_closeFile(hFileIn);

            int waycount = 0;

            if (iResult == 0) {
                distancemap &dimi = pNav->m_mDestinations;

                distancemap::const_iterator it1;
                for (it1 = dimi.begin(); it1 != dimi.end(); ++it1) {
                    waycount += it1->second.size();
                }
            }


            if (iResult == 0) {
                distancemap &dimi = pNav->m_mDestinations;

                distancemap::const_iterator it1;
                for (it1 = dimi.begin(); it1 != dimi.end(); ++it1) {
                    printf("%d\n", it1->first);
                    distlist::const_iterator it2;
                    for (it2 = it1->second.begin(); it2 != it1->second.end(); ++it2) {
                        printf("  %d %f\n", it2->first, it2->second);
                        //                        waycount++;
                    }
                }
            }


            if (iResult == 0) {
                Navigation2 *pNav2 = new Navigation2(pCG);
                pNav2->m_iNumWaterWays = waycount;
                pNav2->m_dSampleDist = pNav->m_dSampleDist;
                pNav2->m_iNumBridges = pNav->m_iNumBridges;

                pNav2->m_mWaterWays = pNav->m_mDestinations;

                if (pNav->m_iNumBridges > 0) {
                    pNav2->m_vBridges = pNav->m_vBridges;
                }

	        hid_t hFileOut; 
                hFileOut = qdf_opencreateFile(sFileOut, 0, 0, "NavCopy", true);

                NavWriter2 *pNW = new NavWriter2(pNav2);
                //iResult = pNW->writeToQDF(sFileOut, 0, 0, "navconvert");
                iResult = pNW->write(hFileOut);
                if (iResult == 0) {
                    printf("success\n");
                } else {
                    printf("coundm't write to xxx[%s]\n", sFileOut.c_str());
                }
            }



            delete pNR;
            //delete pNav;
            delete pCG;
        } else {
            printf("Couldn't open [%s]\n", sFileIn.c_str());
        }
    } else {
        printf("not OK\n");
    }

    return iResult;
}


//----------------------------------------------------------------------------
// main
//
int main(int iArgC, char *apArgV[]) {
    int iResult = 0;

    uint iNumCells       = 0;     // 660492;
    std::string sFileIn  = "";
    std::string sFileOut = "";
    
    iResult = readParams(iArgC, apArgV, &iNumCells, sFileIn, sFileOut);
    if (iResult == 0) {
        printf("FileIn:   [%s]\n", sFileIn.c_str());
        printf("FileOut:  [%s]\n", sFileOut.c_str());
        printf("NumCells: %d\n", iNumCells);
    

        NavToNav2Converter *pNNC = NavToNav2Converter::createInstance(sFileIn, iNumCells);
        if (pNNC != NULL) {
            iResult = pNNC->convert(sFileOut);
            
            if (iResult == 0) {
                printf("success\n");
            }

            delete pNNC;
        }

    } else {
        usage(apArgV[0]);
    }

    return iResult;
}
