#include <cstdio>

#include "xha_strutils.h"
#include "xha_strutilsT.h"
#include "shpUtils.h"
#include "shpHeader.h"


//----------------------------------------------------------------------------
// constructor
//
shpHeader::shpHeader(FILE *fIn) :
    m_fIn(fIn),

    m_iFileLenWords(0),
    m_iFileLen(0),
    m_iVersion(0),
    m_iShapeType(0),
    m_dXMin(0),
    m_dXMax(0),
    m_dYMin(0),
    m_dYMax(0),
    m_dZMin(0),
    m_dZMax(0),
    m_dMMin(0),
    m_dMMax(0) {

}


//----------------------------------------------------------------------------
// read
//
int shpHeader::read() {
    int iResult = -1;
    uchar pBuf[SHP_HEADER_SIZE];

    int iRead = fread(pBuf, 1, SHP_HEADER_SIZE, m_fIn);
    if (iRead == SHP_HEADER_SIZE) {
        uchar *p = pBuf;
        int iMagic;
        p = shpUtils::getNum(p, &iMagic, BIGENDIAN);
        if (iMagic == SHP_MAGIC) {
            // skip unused
            p += SHP_UNUSED_BYTES;
            p = shpUtils::getNum(p, &m_iFileLenWords, BIGENDIAN);
            m_iFileLen = m_iFileLenWords*2;
            if (m_iFileLen > SHP_HEADER_SIZE) {
                p = shpUtils::getNum(p, &m_iVersion, LITTLEENDIAN);
                p = shpUtils::getNum(p, &m_iShapeType, LITTLEENDIAN);
                p = shpUtils::getMBR(p,  m_mbr);
                p = shpUtils::getNum(p, &m_dYMin);
                p = shpUtils::getNum(p, &m_dXMax);
                p = shpUtils::getNum(p, &m_dYMax);
                p = shpUtils::getNum(p, &m_dZMin);
                p = shpUtils::getNum(p, &m_dZMax);
                p = shpUtils::getNum(p, &m_dMMin);
                p = shpUtils::getNum(p, &m_dMMax);
                iResult = 0;
            } else {
                xha_printf("File length in header [%d] smaller than header size [%d]\n", m_iFileLen, SHP_HEADER_SIZE);
            }
        } else {
            xha_printf("Bad Magic: [%08x] instead of [%08x]\n", iMagic, SHP_MAGIC);
        }
        
    } else {
        xha_printf("Only read [%d] instead of [%d] bytes\n", iRead, SHP_HEADER_SIZE);
    }
    return iResult;
}

//----------------------------------------------------------------------------
// display
//
void shpHeader::display(const std::string sCaption) {
    xha_printf("%s\n", sCaption);
    xha_printf("  version:     %d\n", m_iVersion);
    xha_printf("  file length: %d (real %d)\n", m_iFileLenWords, m_iFileLen);
    xha_printf("  shape type:  %d [%s]\n", m_iShapeType, shpUtils::getShapeName(m_iShapeType));
    xha_printf("  x range:    [%f, %f]\n", m_dXMin, m_dXMax);
    xha_printf("  y range:    [%f, %f]\n", m_dYMin, m_dYMax);
    xha_printf("  z range:    [%f, %f]\n", m_dZMin, m_dZMax);
    xha_printf("  m range:    [%f, %f]\n", m_dMMin, m_dMMax);

}
