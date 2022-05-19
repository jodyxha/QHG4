#ifndef __ICOHEADER_H__
#define __ICOHEADER_H__

#include <string>
#include "BufReader.h"
#include "BufWriter.h"

#include "icoutil.h"

// old 
const std::string KEY_START_ICO  = "ICO";
const std::string KEY_START_OCT  = "OCT";
const std::string KEY_START_TET  = "TET";
// new 
const std::string KEY_START_ICO3 = "ICO3";
const std::string KEY_START_OCT3 = "OCT3";
const std::string KEY_START_TET3 = "TET3";

const std::string KEY_ID    = "ID:";
const std::string KEY_LEVEL = "LEVEL:";
const std::string KEY_BOX   = "BOX:";
const std::string KEY_END   = "HEADER_END";

class IcoHeader {
public:
    IcoHeader(int iPolyType, gridtype lID, int iSubLevel, tbox &tBox);
    IcoHeader(int iPolyType, int iSubLevel, tbox &tBox);
    IcoHeader();

    int read(BufReader *pBR);
    int write(BufWriter *pBW);

    gridtype getPolyType() { return m_iPolyType;};
    gridtype getID() { return m_lID;};
    int  getSubLevel() { return m_iSubLevel;};
    void getBox(tbox &tBox);
protected:
    static gridtype createID();
    int m_iPolyType;
    gridtype m_lID;
    int  m_iSubLevel;
    tbox m_tBox;
};

#endif

