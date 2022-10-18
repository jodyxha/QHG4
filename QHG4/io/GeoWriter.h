#ifndef __GEOWRITER_H__
#define __GEOWRITER_H__

class Geography;

class GeoWriter {
public:
    GeoWriter(Geography *pGG);
    virtual ~GeoWriter() {};
    int writeToQDF(const std::string sFileName, int iStep, float fStartTime, const std::string sInfoString);
    int write(hid_t hFile);
    int replace(hid_t hFile);

protected:
    Geography *m_pGG;
    int writeGeoAttributes(hid_t hGeoGroup);
 
};

#endif
