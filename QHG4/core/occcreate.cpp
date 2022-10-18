
#include "types.h"
#include "hdf5.h"
#include <cstring>
#include <string>
#include <cstdio>
#include <cstdlib>

#include "xPopBase.h"
#include "xPopLooper.h"
#include "OccHistory.h"
#include "QDFUtils.h"
#include "xOccTracker.h"

#define NUM_CELLS 5
#define NUM_TIMES 6
int gaganunurere[3][NUM_TIMES][NUM_CELLS] = {

{
    {1,0,0,0,0},
    {1,1,0,0,0},
    {1,0,0,0,0},
    {0,0,0,0,0},
    {0,0,0,0,0},
    {0,0,0,0,0}
},
{
    {0,0,0,0,1},
    {0,1,0,1,1},
    {0,0,0,1,1},
    {0,0,1,1,1},
    {0,1,1,1,1},
    {1,1,1,1,1}
},
{
    {0,0,1,0,0},
    {0,0,1,1,0},
    {0,1,1,0,1},
    {0,1,0,0,0},
    {1,0,0,0,0},
    {0,0,0,0,0}
}
};


//----------------------------------------------------------------------------
// makeOccTracker
//
OccTracker *makeOccTracker(std::vector<std::string> &vPopNames, std::vector<xPopBase *> &vxPops) {
    int iNumCells = NUM_CELLS;
    xPopLooper *pPL = new xPopLooper;
    xPopBase *pGaga = new xPopBase("gaga", iNumCells);

    pPL->addPop(pGaga);
    vxPops.push_back(pGaga);
    vPopNames.push_back("gaga");
    xPopBase *pNunu = new xPopBase("nunu", iNumCells);
    pPL->addPop(pNunu);
    vxPops.push_back(pNunu);
    vPopNames.push_back("nunu");
    xPopBase *pRere = new xPopBase("rere", iNumCells);
    pPL->addPop(pRere);
    vxPops.push_back(pRere);
    vPopNames.push_back("rere");

    std::vector<int> vCellIDs;
    for (int i = 0; i < iNumCells; i++) {
        vCellIDs.push_back(i);
    }
    OccTracker *pOT = OccTracker::createInstance(vCellIDs, pPL);
    return pOT;
}
    
    
//----------------------------------------------------------------------------
// fillPops
//
uchar *fillPops(OccTracker *pOT, std::vector<xPopBase*> &vxPops) {

    int iNumCells = NUM_CELLS;
    uchar *p1 = NULL;

    if (pOT != NULL) {
        
        for (int i = 0; i < NUM_TIMES; i++) {
            for (uint j = 0; j < vxPops.size(); j++) {
                vxPops[j]->setNumAgents(gaganunurere[j][i], iNumCells);
            }            
            pOT->updateCounts(i);
        }
        p1 = pOT->serialize();
    }
    return p1;
}


//----------------------------------------------------------------------------
// writeStringArr
//
int writeStringArr(hid_t hGroup, const char *pAttName, std::vector<std::string> &vStrings) {
    // determine max size
  
    uint iL = 0;
    for (uint i = 0; i < vStrings.size(); i++) {
        if (vStrings[i].length() > iL) {
            iL = vStrings[i].length();
        }
    }
    iL++; // 0-terminated

    // place the strings into the buffer
    // starting each string at amultiple of iL
    char *pStrings = new char[iL*vStrings.size()];
    for (uint i = 0; i < vStrings.size(); i++) {
        strcpy(&(pStrings[i*iL]), vStrings[i].c_str());
    }
    
    hsize_t     dims[1] = {vStrings.size()};
    herr_t      status;
    hid_t hFileType = H5Tcopy (H5T_C_S1);
    status = H5Tset_size (hFileType, iL);
    hid_t hMemType = H5Tcopy (H5T_C_S1);
    status = H5Tset_size (hMemType, iL);
    hid_t hSpace = H5Screate_simple (1, dims, NULL);

    hid_t hAttr = H5Acreate (hGroup, pAttName, hFileType, hSpace, H5P_DEFAULT,
                H5P_DEFAULT);
    status = H5Awrite (hAttr, hMemType, pStrings);

    delete[] pStrings;
    status = H5Aclose (hAttr);
    status = H5Sclose (hSpace);
    status = H5Tclose (hFileType);
    status = H5Tclose (hMemType);

    return (status < 0)?-1:0;
}


//----------------------------------------------------------------------------
// main
//
int main (void) {

    //hid_t hFile = H5Fcreate ("zomboni.qdf", H5F_ACC_TRUNC, H5P_DEFAULT, H5P_DEFAULT);
    hid_t hFile = qdf_createFile("zomboni.qdf", 0, 0, "QHG");
  
    //hid_t hGroup = H5Gcreate(hFile, "/Occupation", H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);
    hid_t hGroup = qdf_createGroup(hFile, "/Occupation");

   std::vector<std::string> vStrings;
   std::vector<xPopBase*> vxPops;
   OccTracker *pOT = makeOccTracker(vStrings, vxPops);
   
   uchar *pBuf = fillPops(pOT, vxPops);
   int iResult =  writeStringArr(hGroup, "PopNames", vStrings);
   if (iResult != 0) {
       printf("had problems\n");
   }
   
   hsize_t dim = pOT->getOccDataSize();
   hid_t hDataSpace = H5Screate_simple(1, &dim, NULL);

   hid_t hDataSet   = H5Dcreate2(hGroup, OCC_DS_OCCTRACK, H5T_NATIVE_UCHAR, hDataSpace, 
                                 H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);
   herr_t status = H5Dwrite(hDataSet, H5T_NATIVE_UCHAR, H5S_ALL, H5S_ALL, H5P_DEFAULT, pBuf);

   qdf_closeGroup(hGroup);
   qdf_closeFile(hFile);

   return  (status < 0)?-1:0;
}

