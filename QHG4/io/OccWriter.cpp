#include <cstdio>
#include <cstring>
#include <hdf5.h>

#include "stdstrutilsT.h"
#include "OccTracker.h"
#include "QDFUtils.h"
#include "QDFUtilsT.h"
#include "OccWriter.h"

//----------------------------------------------------------------------------
// constructor
//
OccWriter::OccWriter(OccTracker *pOcc) 
    : m_pOcc(pOcc) {

}

//----------------------------------------------------------------------------
// writeToHDF
//
int OccWriter::writeToQDF(const std::string sFileName, int iStep, float fStartTime, const std::string sInfoString) {
    int iResult = -1;
    hid_t hFile = qdf_opencreateFile(sFileName, iStep, fStartTime, sInfoString);
    if (hFile > 0) {
        iResult = write(hFile);
        qdf_closeFile(hFile);  
    }
    return iResult;
}

//----------------------------------------------------------------------------
// write
//
int OccWriter::write(hid_t hFile) {
    int iResult = -1;

    if (m_pOcc != NULL) {
        iResult = 0;
        stdprintf("Occwriter creating group [%s]\n", OCCGROUP_NAME);
        hid_t hOccGroup = qdf_opencreateGroup(hFile, OCCGROUP_NAME);
        if (hOccGroup > 0) {
            writeOccAttributes(hOccGroup);

            uchar *pOccData = m_pOcc->serialize();
            int iOccDataSize = m_pOcc->getOccDataSize();
            if (iResult == 0) {
                iResult = qdf_writeArray(hOccGroup, OCC_DS_OCCTRACK, iOccDataSize, pOccData);
            }
            
            
            if (iResult == 0) {
                //                stdprintf("[OccWriter] data written\n");
            } else {
                stdprintf("[NavWriter] error writing arrays\n");
            }
            
            delete[] pOccData;


            qdf_closeGroup(hOccGroup);
            

        } else {
            iResult = -1;
            stdprintf("[OccWriter] Couldn't open group [%s]\n", OCCGROUP_NAME);
            // couldn't open group
        }
    } else {
        stdprintf("[OccWriter] No OccTracker set\n");
    }
    return iResult;
}

 
//----------------------------------------------------------------------------
// writeOccAttributes
//
int OccWriter::writeOccAttributes(hid_t hOccGroup) {

    const stringvec &vStrings = m_pOcc->getPopNames();
    
    // maybe save the pop names here instead of inside the data 
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
    
    herr_t      status;
    hid_t hFileType = H5Tcopy (H5T_C_S1);
    status = H5Tset_size (hFileType, iL);
    hid_t hMemType = H5Tcopy (H5T_C_S1);
    status = H5Tset_size (hMemType, iL);

    status = qdf_insertSAttribute(hOccGroup, OCC_ATTR_POP_NAMES, pStrings);

    delete[] pStrings;
    status = H5Tclose (hFileType);
    status = H5Tclose (hMemType);

    return (status < 0)?-1:0;
}
