#ifndef __OCCWRITER_H__
#define __OCCWRITER_H__

class OccTracker;

class OccWriter {
public:
    OccWriter(OccTracker *pOcc);
    int writeToQDF(const std::string sFileName, int iStep, float fStartTime, const std::string sInfoString);
    int write(hid_t hFile);

protected:
    OccTracker *m_pOcc;
    int writeOccAttributes(hid_t hOccGroup);
};

#endif
