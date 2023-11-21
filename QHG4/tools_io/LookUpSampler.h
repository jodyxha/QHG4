#ifndef __LOOKUPSAMPLER_H__
#define __LOOKUPSAMPLER_H__

#include <string>

#include "LookUp.h"
#include "PNGImage.h"

class LookUpSampler {
public:
    static LookUpSampler *createInstance(uint iW, uint iH, std::string sLookUpName);
    ~LookUpSampler();
    
    int makeImage(std::string sOutputName, uint iBorder);
    
protected:
    LookUpSampler(uint iW, uint iH);
    int init(std::string sLookUpName);
    int findLookUp(std::string sLookUpName);
    void createArrays();
    void destroyArrays();
    int fillDataArray();
    int fillPNGArray();
    int insertBorder(uint iBorder);


    uint m_iW;
    uint m_iH;
    int  m_iLU;
    LookUp *m_pLookUp;

    double **m_ppData;
    uchar  **m_ppPNGData;
    PNGImage *m_pPNGImage;
};


#endif
