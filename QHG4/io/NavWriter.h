#ifndef __NAVWRITER_H__
#define __NAVWRITER_H__

class Navigation;

class NavWriter {
public:
    NavWriter(Navigation *pNav);
    int writeToQDF(const std::string sFileName, int iStep, float fStartTime, const std::string sInfoString);
    int write(hid_t hFile);

protected:
    Navigation *m_pNav;
    int writeNavAttributes(hid_t hNavGroup);
};

#endif
