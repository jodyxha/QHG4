#include <cstdio>
#include <cstring>
#include <hdf5.h>

#include "types.h"
#include "QDFUtils.h"
#include "QDFUtilsT.h"

#include "QDFSequenceExtractor.h"
#include "QDFSequenceExtractor.cpp"
#include "QDFPhenomeExtractor2.h"


//----------------------------------------------------------------------------
// createInstance
//  use if the population file contains geo info
//  pQDFPopFile :     Population QDF file
//  pSpeciesName :    Name of species for which to extract genomes
//                    (if NULL, first species will be used)
//  pAttrGenomeSize : name of genome size attribute
//  pDataSetGenome :  name of genome dataset
//
QDFPhenomeExtractor2 *QDFPhenomeExtractor2::createInstance(const char *pQDFPopFile, 
                                                           const char *pSpeciesName, 
                                                           const char *pAttrPhenomeSize,
                                                           const char *pAttrPloidy,
                                                           const char *pDataSetPhenome,
                                                           WELL512 *pWELL,
                                                           bool bCartesian) {
    return createInstance(pQDFPopFile, pQDFPopFile, pSpeciesName, pAttrPhenomeSize, pAttrPloidy, pDataSetPhenome, pWELL, bCartesian);
}


//----------------------------------------------------------------------------
// createInstance
//  use if the population file does not contain geo info
//  pQDFPopFile :     Population QDF file
//  pSpeciesName :    Name of species for which to extract genomes
//                    (if NULL, first species will be used)
//  pAttrGenomeSize : name of genome size attribute
//  pDataSetGenome :  name of genome dataset
//
QDFPhenomeExtractor2 *QDFPhenomeExtractor2::createInstance(const char *pQDFGeoFile,
                                                           const char *pQDFPopFile,
                                                           const char *pSpeciesName, 
                                                           const char *pAttrPhenomeSize,
                                                           const char *pAttrPloidy,
                                                           const char *pDataSetPhenome,
                                                           WELL512 *pWELL,
                                                           bool bCartesian) {
   
    QDFPhenomeExtractor2 *pQPC = new QDFPhenomeExtractor2(pWELL, bCartesian); // 1: only one set per individual
    int iResult = pQPC->init(pQDFGeoFile, pQDFPopFile, pSpeciesName, pAttrPhenomeSize, pAttrPloidy, pDataSetPhenome);
    if (iResult != 0) {
        delete pQPC;
        pQPC = NULL;
    }
    return pQPC;
}



//----------------------------------------------------------------------------
// constructor
//
QDFPhenomeExtractor2::QDFPhenomeExtractor2(WELL512 *pWELL, bool bCartesian)
    : QDFSequenceExtractor(pWELL, bCartesian) {
}

//----------------------------------------------------------------------------
// init
//
int QDFPhenomeExtractor2::init(const char *pQDFGeoFile, 
                               const char *pQDFPopFile, 
                               const char *pSpeciesName, 
                               const char *pAttrPhenomeSize,
                               const char *pAttrPloidy,
                               const char *pDataSetPhenome) {


    int iResult = -1;

    m_pAttrPloidy = new char[strlen(pAttrPloidy)+1];
    strcpy(m_pAttrPloidy, pAttrPloidy);

    iResult = QDFSequenceExtractor::init(pQDFGeoFile, pQDFPopFile, pSpeciesName, pAttrPhenomeSize, pDataSetPhenome);
   
    return iResult;
}


//----------------------------------------------------------------------------
// extractAdditionalAttributes
//
int QDFPhenomeExtractor2::extractAdditionalAttributes() {
    int iResult =-1;

    iResult = qdf_extractAttribute(m_hSpecies, m_pAttrPloidy, 1, &m_iPloidy);
    if (iResult != 0) {
        fprintf(stderr, "WARNING: attribute [%s] not found; using %d\n", m_pAttrPloidy, 1);
        iResult = 0;
    }
    return iResult;

}


//----------------------------------------------------------------------------
// calcNumBlocks
//
int QDFPhenomeExtractor2::calcNumBlocks() {
    return /*m_iPloidy* */m_iSequenceSize;
}


//----------------------------------------------------------------------------
// calcNumItemsInBlock
//
int QDFPhenomeExtractor2::calcNumItemsInBlock() {
    return 1;
}

//----------------------------------------------------------------------------
// readQDFSequenceSlab
//
hid_t QDFPhenomeExtractor2::readQDFSequenceSlab(hid_t hDataSet, 
                                               hid_t hMemSpace, 
                                               hid_t hDataSpace, 
                                               float *aBuf) {
    return  H5Dread(hDataSet, H5T_NATIVE_FLOAT, hMemSpace, hDataSpace, H5P_DEFAULT, aBuf);
}
