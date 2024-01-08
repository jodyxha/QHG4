#ifndef __NAVWRITER2_H__
#define __NAVWRITER2_H__

class Navigation2;

class NavWriter2 {
public:
    NavWriter2(Navigation2 *pNav);
    int writeToQDF(const std::string sFileName, int iStep, float fStartTime, const std::string sInfoString);
    int write(hid_t hFile);

protected:
    Navigation2 *m_pNav;
    int writeNavAttributes(hid_t hNavGroup);
};

#endif
