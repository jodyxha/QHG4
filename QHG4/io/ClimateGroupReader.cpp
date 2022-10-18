#include <cstdio>
#include <cstring>
#include <hdf5.h>

#include "Climate.h"
#include "QDFUtils.h"
#include "QDFUtilsT.h"
#include "GroupReader.h"
#include "GroupReader.cpp"
#include "ClimateGroupReader.h"

//----------------------------------------------------------------------------
// constructor
//
ClimateGroupReader::ClimateGroupReader() {
}


//----------------------------------------------------------------------------
// createClimateGroupReader
//
ClimateGroupReader *ClimateGroupReader::createClimateGroupReader(const std::string sFileName) {
    ClimateGroupReader *pGR = new ClimateGroupReader();
    int iResult = pGR->init(sFileName, CLIGROUP_NAME);
    if (iResult != 0) {
        delete pGR;
        pGR = NULL;
    }
    return pGR;
}

//----------------------------------------------------------------------------
// createClimateGroupReader
//
ClimateGroupReader *ClimateGroupReader::createClimateGroupReader(hid_t hFile) {
    ClimateGroupReader *pGR = new ClimateGroupReader();
    int iResult = pGR->init(hFile, CLIGROUP_NAME);
    if (iResult != 0) {
        delete pGR;
        pGR = NULL;
    }
    return pGR;
}



//----------------------------------------------------------------------------
// tryReadAttributes
//
int ClimateGroupReader::tryReadAttributes(ClimateAttributes *pAttributes) {
    int iResult = GroupReader<Climate, ClimateAttributes>::tryReadAttributes(pAttributes);


    if (iResult == 0) {
        iResult = qdf_extractAttribute(m_hGroup, CLI_ATTR_NUM_SEASONS, 1,  &(pAttributes->m_iNumSeasons)); 
    }


    return iResult;
}



//-----------------------------------------------------------------------------
// readArray
//
int ClimateGroupReader::readArray(Climate *pCG, const std::string sArrayName) {
    int iResult = -1;
    // before loading the array, make sure that the essential variables correspond
    if ((m_pAttributes == NULL) ||
        (m_pAttributes->m_iNumCells == pCG->m_iNumCells)) {
        iResult = 0;
    } else {
        iResult = -1;
        if (m_pAttributes != NULL) {
            stdprintf("Number of cells does not correspond:\n");
            stdprintf("  ClimateGroupReader::m_iNumCells: %d; Climate::m_iNumCells: %d\n", m_pAttributes->m_iNumCells,  pCG->m_iNumCells);
        }
    }
    
    if (iResult == 0) {
        if (sArrayName == CLI_DS_ACTUAL_TEMPS) {
            iResult = qdf_readArray(m_hGroup, CLI_DS_ACTUAL_TEMPS,   pCG->m_iNumCells, pCG->m_adActualTemps);
        } else if (sArrayName == CLI_DS_ACTUAL_RAINS) {
            iResult = qdf_readArray(m_hGroup, CLI_DS_ACTUAL_RAINS,   pCG->m_iNumCells, pCG->m_adActualRains);
        } else if (sArrayName == CLI_DS_ANN_MEAN_TEMP) {
            iResult = qdf_readArray(m_hGroup, CLI_DS_ANN_MEAN_TEMP,  pCG->m_iNumCells, pCG->m_adAnnualMeanTemp);
        } else if (sArrayName == CLI_DS_ANN_TOT_RAIN) {
            iResult = qdf_readArray(m_hGroup, CLI_DS_ANN_TOT_RAIN,   pCG->m_iNumCells, pCG->m_adAnnualRainfall);
        } else {
            stdprintf("Unknown array [%s]\n", sArrayName);
            iResult = -1;
        }
    }
    return iResult;
}


//-----------------------------------------------------------------------------
// readData
//
int ClimateGroupReader::readData(Climate *pCG) {
    int iResult = -1;
    // before loading the array, make sure that the essential variables correspond
    if ((m_pAttributes == NULL) ||
        (m_pAttributes->m_iNumCells == pCG->m_iNumCells)) {
        iResult = 0;
    } else {
        iResult = -1;
        if (m_pAttributes != NULL) {
            stdprintf("Number of cells or max neighbors do not correspond:\n");
            stdprintf("  ClimateGroupReader::m_iNumCells: %d; Climate::m_iNumCells: %d\n", m_pAttributes->m_iNumCells,  pCG->m_iNumCells);
        }
    }

    if (iResult == 0) {
        iResult = qdf_readArray(m_hGroup, CLI_DS_ACTUAL_TEMPS,   pCG->m_iNumCells, pCG->m_adActualTemps);
    }
    if (iResult == 0) {
        iResult = qdf_readArray(m_hGroup, CLI_DS_ACTUAL_RAINS,   pCG->m_iNumCells, pCG->m_adActualRains);
    }
    if (iResult == 0) {
        iResult = qdf_readArray(m_hGroup, CLI_DS_ANN_MEAN_TEMP,  pCG->m_iNumCells, pCG->m_adAnnualMeanTemp);
    }
    if (iResult == 0) {
        iResult = qdf_readArray(m_hGroup, CLI_DS_ANN_TOT_RAIN,   pCG->m_iNumCells, pCG->m_adAnnualRainfall);
    }

    if (pCG->m_iNumSeasons > 1) {
        if (iResult == 0) {
            iResult = qdf_readArray(m_hGroup, CLI_DS_SEAS_TEMP_DIFF, m_iNumCells*pCG->m_iNumSeasons, pCG->m_adSeasTempDiff);
        }
        if (iResult == 0) {
            iResult = qdf_readArray(m_hGroup, CLI_DS_SEAS_RAIN_RAT,  m_iNumCells*pCG->m_iNumSeasons, pCG->m_adSeasRainRatio);
        }
        if (iResult == 0) {
            iResult = qdf_readArray(m_hGroup, CLI_DS_CUR_SEASON,   1, &(pCG->m_iCurSeason));
        }
 
    }

    pCG->m_bUpdated = true;
    pCG->notifyObservers(EVT_CLIMATE_CHANGE, NULL);
    return iResult;


}
