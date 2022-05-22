#include <cstdio>
#include <cstring>
#include <omp.h>

#include "strutils.h"
#include "stdstrutilsT.h"

#include "MedianHybMaker.h"
#include "MedianHybMaker.cpp"

typedef struct aginfo_ymt {
    aginfo_ymt(): m_ulID(0), m_ulCellID(0), m_iGender(0), m_fHybridization(0), m_iYchr(0), m_imtDNA(0) {};
    aginfo_ymt(idtype ulID, gridtype ulCellID, uchar iGender, float fHyb, uchar iYchr, uchar imtDNA): m_ulID(ulID), m_ulCellID(ulCellID), m_iGender(iGender), m_fHybridization(fHyb), m_iYchr(iYchr), m_imtDNA(imtDNA) {};
    idtype   m_ulID;
    gridtype m_ulCellID;
    uchar    m_iGender;
    float    m_fHybridization;
    uchar    m_iYchr;
    uchar    m_imtDNA;
} aginfo_ymt;
    
// agent structure for simple hybridisation
typedef struct aginfo_hyb {
    idtype   m_ulID;
    gridtype m_ulCellID;
    float    m_fHybridization;
} aginfo_hyb;




void usage(const std::string sApp) {
    stdprintf(" %s - calculating hybridisation medians\n", sApp);
    stdprintf("usage:\n");
    stdprintf("   %s <qdf-file>:<species> <type-name>\n", sApp);
    stdprintf("where\n");
    stdprintf("  qdf-file   a qdf file containing agents\n");
    stdprintf("  species    species name\n");
    stdprintf("  type-name  'hyb' or 'ymt'\n");
}


int main(int iArgC, char *apArgV[]) {
    int iResult = 0;
    if (iArgC > 2) {
        stringvec v;
        uint iNum = splitString(apArgV[1], v, ":", false);
        if (iNum == 2) {
            std::string sQDFFile{v[0]};
            std::string sSpecies{v[1]};
            std::string sType{apArgV[2]};
            
            MedianHybMakerBase *pMHMB = NULL;
            if (sType == "hyb") {
                pMHMB = MedianHybMaker<aginfo_hyb>::create_instance(sQDFFile, sSpecies);
            } else if (sType == "ymt") {
                pMHMB = MedianHybMaker<aginfo_ymt>::create_instance(sQDFFile, sSpecies);
            } else {
                stdprintf("expected 'hyb' or 'ymt' as type-name\n");
                iResult = -1;
            }

            if (iResult == 0) {
                if (pMHMB != NULL) {
                    iResult = pMHMB->calc_medians();
                    if (iResult == 0) {
                        stdprintf("+++ success +++\n");
                    } else {
                        stdprintf("--- success ---\n");
                    }
                    
                    pMHMB->write_medians();

                    float *pBuf = pMHMB->getMedians();
                    for (uint i = 0; i < 2000; i++) {
                        stdprintf("%f ", pBuf[i]);
                        if (i%20 == 0) {
                            stdprintf("\n");
                        }
                    }
                    stdprintf("\n");

                    delete pMHMB;
                } else {
                    stdprintf("couldn't create MedianHybMaker\n");
                    iResult = -1;
                }
            }
        } else {
            usage(apArgV[0]);
            stdprintf("expected <qdf-file>:<species> as first argument\n");
            iResult = -1;
        }
    } else {
        usage(apArgV[0]);
        iResult = -1;
    }

    return iResult;
}

