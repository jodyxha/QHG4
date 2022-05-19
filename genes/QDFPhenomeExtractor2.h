#ifndef __QDF_PHENOME_EXTRACTOR2_H__
#define __QDF_PHENOME_EXTRACTOR2_H__


#include <hdf5.h>
#include "types.h"

#include "SequenceProvider.h"
#include "QDFSequenceExtractor.h"

class QDFPhenomeExtractor2 : public QDFSequenceExtractor<float> {
public:  
    static QDFPhenomeExtractor2 *createInstance(const char *pQDFFile, 
                                                const char *pSpeciesName, 
                                                const char *pAttrPhenomeSize,
                                                const char *pAttrPloidy,
                                                const char *pDataSetPhenome,
                                                WELL512 *pWELL,
                                                bool bCartesian=false);
    
    static QDFPhenomeExtractor2 *createInstance(const char *pGeoFile, 
                                                const char *pQDFFile, 
                                                const char *pSpeciesName, 
                                                const char *pAttrPhenomeSize,
                                                const char *pAttrPloidy,
                                                const char *pDataSetPhenome,
                                                WELL512 *pWELL,
                                                bool bCartesian=false);

    virtual ~QDFPhenomeExtractor2() {};

    
protected:
    QDFPhenomeExtractor2(WELL512 *pWELL, bool bCartesian);

    int init(const char *pQDFGeoFile, 
             const char *pQDFPopFile, 
             const char *pSpeciesName, 
             const char *pAttrPhenomeSize,
             const char *pAttrPloidy,
             const char *pDataSetPhenome);

    virtual int extractAdditionalAttributes();
    virtual int calcNumBlocks();
    virtual int calcNumItemsInBlock();
    virtual hid_t readQDFSequenceSlab(hid_t hDataSet, hid_t hMemSpace, hid_t hDataSpace, float *aBuf);

    char  *m_pAttrPloidy;

};


#endif

