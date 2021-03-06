#ifndef __CLIMATEWRITER_H__
#define __CLIMATEWRITER_H__

class Climate;

class ClimateWriter {
public:
    ClimateWriter(Climate *pC);
    int writeToQDF(const std::string sFileName, int iStep, float fStartTime, const std::string sInfoString);
    int write(hid_t hFile);

protected:
    Climate *m_pC;
    int writeClimateAttributes(hid_t hClimateGroup);
 
};

#endif
