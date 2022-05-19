#ifndef __SEQUENCEIOUTILS_H__
#define __SEQUENCEIOUTILS_H__

#include <hdf5.h>
#include <vector>
#include "LBBase.h"

#include "LayerArrBuf.h"
#include "LBController.h"



template<typename T>
class SequenceIOUtils {

public:
    static SequenceIOUtils *createInstance(const char *pDataSetName, 
                                           hid_t hdf_type, 
                                           LayerArrBuf<T> *paSequence,
                                           LBController *pSequenceController,
                                           std::vector<int> *pvDeadList,
                                           uint iBlockSize);

    
    int writeSequenceDataQDF(hid_t hSpeciesGroup, uint iNumArrays);
    int readSequenceDataQDF(hid_t hSpeciesGroup,  uint iNumReadItems);

    int dumpSequenceDataQDF(hid_t hSpeciesGroup);
    int restoreSequenceDataQDF(hid_t hSpeciesGroup);

    virtual ~SequenceIOUtils();
protected:
    SequenceIOUtils(const char *pDataSetName, 
                    hid_t hdf_type, 
                    LayerArrBuf<T> *paSequence,
                    LBController *pSequenceController,
                    std::vector<int> *pvDeadList,
                    uint iBlockSize);
    int init();


    const char     *m_pDataSetName;
    hid_t           m_hdf_type;
    LayerArrBuf<T> *m_paSequence;
    LBController   *m_pSequenceController;
    LayerArrBuf<T>  m_aWriteCopy; 
    LBController   *m_pWriteCopyController;
    uint            m_iLayerSize;
    uint            m_iBlockSize;

    std::vector<int> *m_pvDeadList;

};


#endif

