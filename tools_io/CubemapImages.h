#ifndef __CUBEMAPIMAGES_H__
#define __CUBEMAPIMAGES_H__

#include "types.h"
#include "EQsahedron.h"

class CubemapImages {
public:
    static CubemapImages*createInstance(std::string sInputQDF, std::string sArrayPath, uint iSize);

    virtual ~CubemapImages();
 
    int createImages(std::string sOutBody);
 
protected:
    CubemapImages(uint iSize);
    int init(std::string sInputQDF, std::string sArrayPath);
    int readArray(std::string sInputQDF, std::string sArrayPath); 
        
    int getValues(uint *pActiveCoords, int iClampVal);
    
    uint m_iNumCells;
    uint m_iNumSubDivs;
    uint m_iSize;
    double *m_pData;
    double **m_pImgData;
    EQsahedron *m_pEQ;
};
#endif
