/*============================================================================
| PolyLine
| 
|  representation of a piecewise linear function
|  
|  Author: Jody Weissmann
\===========================================================================*/ 

#include <cstring>
#include <string>
#include <cstdlib>
#include "qhg_consts.h"
#include "stdstrutilsT.h"
#include "PolyLine.h"

PolyLine::PolyLine(unsigned int iNumSegments) 
:   m_iNumSegments(iNumSegments),
    m_afX(NULL),
    m_afV(NULL),
    m_afA(NULL) {
    
    if (m_iNumSegments > 0) {
        m_afX = new double[(m_iNumSegments+1)*sizeof(double)];
        memset(m_afX, 0, (m_iNumSegments+1)*sizeof(double));
        m_afV = new double[(m_iNumSegments+1)*sizeof(double)];
        memset(m_afV, 0, (m_iNumSegments+1)*sizeof(double));
        m_afA = new double[m_iNumSegments*sizeof(double)];
        memset(m_afA, 0, m_iNumSegments*sizeof(double));
    }
}

PolyLine::PolyLine(PolyLine *pPL) 
:   m_iNumSegments(pPL->m_iNumSegments),
    m_afX(NULL),
    m_afV(NULL),
    m_afA(NULL) {

    if (m_iNumSegments > 0) {
        m_afX = new double[(m_iNumSegments+1)*sizeof(double)];
        memcpy(m_afX, pPL->m_afX, (m_iNumSegments+1)*sizeof(double));
        m_afV = new double[(m_iNumSegments+1)*sizeof(double)];
        memcpy(m_afV, pPL->m_afV, (m_iNumSegments+1)*sizeof(double));
        m_afA = new double[m_iNumSegments*sizeof(double)];
        memcpy(m_afA, pPL->m_afA, m_iNumSegments*sizeof(double));
    }
}

PolyLine::~PolyLine() {
    if (m_afX != NULL) {
        delete[] m_afX;
    }
    if (m_afV != NULL) {
        delete[] m_afV;
    }
    if (m_afA != NULL) {
        delete[] m_afA;
    }
}

double PolyLine::getVal(double fX) {
    double fV = 0;
    unsigned int i = 0;
    double *pf = m_afX;

    if (m_iNumSegments > 0) {
        // if fX is too big, set highest value
        if (fX >= m_afX[m_iNumSegments]) {
            fV = m_afV[m_iNumSegments];
        } else {
            // find  starting point for segment
            while ((i <= m_iNumSegments) && (fX > *pf)) {
                ++i; ++pf;
            }
        
            // num values = num segs +1
            if (i == 0) {
                fV = m_afV[0];
            } else if (i <= m_iNumSegments) {
                fV = m_afV[i-1] + m_afA[i-1]*(fX - m_afX[i-1]);
            } else {
                // this should not happen
                fV = m_afV[m_iNumSegments];
            }
        }
    } else {
        fV = fX;
    }
    return fV;
}       


PolyLine * PolyLine::readFromString(const std::string sData) {
    PolyLine *pPL = NULL;
    std::vector<double> vecData;
    bool bOK = true;
    
    stringvec vParts;
    uint iNum = splitString(sData, vParts, " \t");
    if ((iNum > 0) && ((iNum%2)==0)){
        for (uint i = 0; bOK && (i < iNum); i+=2) {
            double fX;
            double fV;
            if (strToNum(vParts[i], &fX) && strToNum(vParts[i+1], &fV)) {
                vecData.push_back(fX);
                vecData.push_back(fV);
                bOK = true;
            } else {
                bOK = false;
            }
        }
    } else {
        stdprintf("[PolyLine::readFromString] Expected non-zero even number of argumend : [%s]", sData);
        bOK = false;
    }
    
    if (vecData.size() > 0) {
        if (bOK && (vecData.size() > 2)) {
            pPL = new PolyLine((unsigned int)vecData.size()/2-1);
            for (unsigned int i = 0; i < vecData.size()/2; i++) {
                pPL->addPoint(i, vecData[2*i], vecData[2*i+1]); 
            }
        } else {
            stdprintf("Bad Function def (number format) : [%s] or not enough data points (%zd)\n", sData, vecData.size());
        } 
    }
    return pPL;
}



void PolyLine::write(FILE *fOut) {
    if (m_iNumSegments > 0) {
        for (unsigned int j = 0; j < m_iNumSegments+1; j++) {
            fprintf(fOut, "%f %f ", m_afX[j], m_afV[j]);
        }
    }
    fprintf(fOut, "\n");
}



void PolyLine::display(const char *pIndent, const std::string sCaption) {
    stdprintf("%s%s [%d]:\n%s  ", pIndent, sCaption, m_iNumSegments, pIndent);
    if (m_iNumSegments > 0) {
        for (unsigned int j = 0; j < m_iNumSegments+1; j++) {
            stdprintf("%f %f ", m_afX[j], m_afV[j]);
        }
    }
    stdprintf("\n");
}
