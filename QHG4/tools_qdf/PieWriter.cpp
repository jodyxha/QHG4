#include <cmath>
#include <cstring>

#include "hdf5.h"

#include "strutils.h"
#include "xha_strutilsT.h"

#include "QDFUtils.h"
#include "PieWriter.h"

const std::string PIEGROUP_NAME      = "PiePlots";
const std::string PIE_ATTR_NUQ_PIES  = "NumPies";
const std::string PIE_ATTR_NUM_VALS  = "NumVals";
const std::string PIE_ATTR_NUM_DIMS  = "NumDims";
const std::string PIE_DATASET_NAME   = "PieDataSet";
const std::string PIE_ATTR_VAL_NAMES = "ValNames";


//----------------------------------------------------------------------------
//  constructor
//
PieWriter::PieWriter(const std::string sPieName, uint iNumVals, uint iNumDims) 
    : m_iNumPies(0),
      m_iNumVals(iNumVals),
      m_iNumDims(iNumDims),
      m_sPieName(sPieName),
      m_sValNames(""),
      m_pAllData(NULL),
      m_bSpherical(true),
      m_bPointsOK(false),
      m_bVerbose(false) {

}


//----------------------------------------------------------------------------
//  destructor
//
PieWriter::~PieWriter() {
    if (m_pAllData != NULL) {
        delete[] m_pAllData;
    }
}


//----------------------------------------------------------------------------
//  createInstance
//
PieWriter *PieWriter::createInstance(const std::string sPieName, maphistos &mHistos, uint iNumBins, uint iNumDims) {
    PieWriter *pPW = new PieWriter(sPieName, iNumBins, iNumDims);
    int iResult = pPW->init(mHistos);
    if (iResult != 0) {
        delete pPW;
        pPW = NULL;
    }
    return pPW;
}


//----------------------------------------------------------------------------
//  init
//    -sanity checks
//    - extract useful information from  grouped vals
//
int PieWriter::init(maphistos &mHistos) {
    int iResult = 0;


    m_mHistos = mHistos;
    // number of pies
    m_iNumPies = (uint) m_mHistos.size();
    if (m_bVerbose) xha_printf("num pies: %d\n", m_iNumPies);
    
    if (iResult == 0) {
        /*
        // number of values
        m_iNumVals = (uint) iNumVals;
        xha_printf("num vals: %d\n", m_iNumVals);
        */
        // create default value names ("item_XXX")
        int iNumDigits = 1+(int)(log(m_iNumVals)/log(10));
        if (m_bVerbose) xha_printf("numvals %d, numdigits %d\n", m_iNumVals, iNumDigits);
        m_sValNames="";
        for (uint i = 0; i < m_iNumVals; i++) {
            std::string s0 = std::to_string(i);
            //printf("valnames [%d]:[%s]\n",i, s0.c_str());
            std::string s1(iNumDigits - s0.size(), '0');
            if (i>0) {
                s1 = ";"+s1;
            }
            m_sValNames += s1+s0;
        }

    }
    
    return iResult;
}


//----------------------------------------------------------------------------
//  setValueNames
//    - extract useful information from  grouped vals
//
int PieWriter::setValueNames(const stringvec &svValueNames) {
    int iResult = -1;
    
    if (svValueNames.size() == m_iNumVals) {
        std::string sFinal = "";
        for (uint i = 0; i < svValueNames.size(); i++) {
            if (i > 0) {
                sFinal += ";";
            }
            sFinal += svValueNames[i];
        }
        m_sValNames = sFinal;
        iResult = 0;
    } else {
        xha_printf("Exactly %u value names are required, not %zd\n", m_iNumVals, svValueNames.size());
    }

    return iResult;
}


//----------------------------------------------------------------------------
//  prepareData
//    if the points and norms are provided manually
//
int PieWriter::prepareData(std::map<int, pointnorm> mPointsNorms) {
    int iResult = 0;
    // check if necessary IDs are there
    //    typename groupedvals<double>::const_iterator it;
    maphistos::const_iterator it;
    
    for (it = m_mHistos.begin(); (iResult == 0) && (it != m_mHistos.end()); ++it) {
        std::map<int, pointnorm>::const_iterator it2 = mPointsNorms.find(it->first);
        if (it2 == mPointsNorms.end()) {
            iResult = -1;
            xha_printf("ID [%d] is missing from provided pointnorm map\n", it->first);
        }
    }

    if (iResult == 0) {
        m_mPointsNorms = mPointsNorms;
        m_bPointsOK = true;
        iResult = prepareDataReal();
    } 
    return iResult;
}

//----------------------------------------------------------------------------
//  prepareData
//    if the points and norms are calculated from coordinates 
//
int PieWriter::prepareData(int iNumCoords, double *dLons, double *dLats, bool bSpherical, double dRadius) {
    int iResult = 0;

    m_bSpherical = bSpherical;
    m_mPointsNorms.clear();

    //    typename groupedvals<double>::const_iterator it;
    maphistos::const_iterator it;
    
    //    for (it = m_mGroups.begin(); (iResult == 0) && (it != m_mGroups.end()); ++it) {
    for (it = m_mHistos.begin(); (iResult == 0) && (it != m_mHistos.end()); ++it) {
        double dLon = dLons[it->first];
        double dLat = dLats[it->first];
        
        

        pointnorm pn;
        double adPos[3];
        double adNorm[3];
        if (m_bSpherical) {
            dLon *= Q_PI/180;
            dLat *= Q_PI/180;
           
            adPos[0] = dRadius*cos(dLon)*cos(dLat);
            adPos[1] = dRadius*sin(dLon)*cos(dLat);
            adPos[2] = dRadius*sin(dLat);
            memcpy(adNorm, adPos, 3*sizeof(double));
        } else {
            adPos[0] = dLon;
            adPos[1] = dLat;
            adPos[2] = 0.0;
            adNorm[0] = 0.0;
            adNorm[1] = 0.0;
            adNorm[2] = 1.0;
        }
        memcpy(pn.m_vPos,  adPos, 3*sizeof(double));
        memcpy(pn.m_vNorm, adNorm, 3*sizeof(double));
        
        m_mPointsNorms[it->first] = pn;
    }
    m_bPointsOK = true; 
    iResult = prepareDataReal();
    return iResult;
}


//----------------------------------------------------------------------------
//  prepareDataReal
//    do the actual data preparation
//
int PieWriter::prepareDataReal() {
    int iResult = 0;


    if (m_bPointsOK) {
        m_pAllData = new double[m_iNumPies*(6+m_iNumVals)];
        double *pCur = m_pAllData;
        double *pH = new double[m_iNumVals];

        maphistos::const_iterator it;
        for (it = m_mHistos.begin(); (iResult == 0) && (it != m_mHistos.end()); ++it) {
            for(uint k = 0; k < m_iNumVals; ++k) {
                pH[k] = (long)it->second[k]; 
            }
            const pointnorm &pn = m_mPointsNorms [it->first];
            memcpy(pCur, pn.m_vPos,  3*sizeof(double)); 
            pCur += 3;
            memcpy(pCur, pn.m_vNorm, 3*sizeof(double)); 
            pCur += 3;
            memcpy(pCur, pH, m_iNumVals*sizeof(double));
            pCur += m_iNumVals;
        }

        delete[] pH;
    } else {
        xha_printf("One of the pPrepareData(...) methods must be called berfore prepareDataReal()\n");
        iResult = -1;
    }
    return iResult;
}


//----------------------------------------------------------------------------
//  writeToQDF
//    do the actual data preparation
//
int PieWriter::writeToQDF(const std::string sQDFFile) {

    int iResult = -1;

    hid_t hFile = qdf_openFile(sQDFFile.c_str(), "r+");
    if (hFile != H5P_DEFAULT) {
        //xha_printf("opened file\n");
        if (qdf_link_exists(hFile, PIEGROUP_NAME)) {
            // a pie group already exists, so we can use it
                
        } else {
            qdf_createGroup(hFile, PIEGROUP_NAME);
        }

        hid_t hPieGroup = qdf_openGroup(hFile, PIEGROUP_NAME, true);
        if (hPieGroup != H5P_DEFAULT) {
            //xha_printf("opened pie group\n");

            if (!qdf_link_exists(hPieGroup, m_sPieName)) {
                // xha_printf("opened sub group [%s]\n", m_sPieName);
                hid_t hPieItem = qdf_createGroup(hPieGroup, m_sPieName);
                if (hPieItem != H5P_DEFAULT) {
                    // now we add some attributes

                    qdf_insertAttribute(hPieItem,  PIE_ATTR_NUQ_PIES,  1, &m_iNumPies);
                    qdf_insertAttribute(hPieItem,  PIE_ATTR_NUM_VALS,  1, &m_iNumVals);
                    qdf_insertAttribute(hPieItem,  PIE_ATTR_NUM_DIMS,  1, &m_iNumDims);
                    qdf_insertSAttribute(hPieItem, PIE_ATTR_VAL_NAMES,     m_sValNames);

                    if (m_bVerbose) xha_printf("written attributes\n");
                    // now let's add the data

                    if (m_bVerbose) xha_printf("writing array of %d elements\n", m_iNumPies*(6+m_iNumVals));
                    qdf_writeArray(hPieItem, PIE_DATASET_NAME, m_iNumPies*(6+m_iNumVals), m_pAllData);

                    qdf_closeGroup(hPieItem);
                    iResult = 0;
                } else {
                    xha_printf("Couldn't open subgroup [%s]\n", m_sPieName);
                }
                
            } else {
                xha_printf("Pie subgroup [%s] already exists\n", m_sPieName);
            }

        } else {
            xha_printf("Couldn't open group [%s]\n", PIEGROUP_NAME);
        }
        qdf_closeFile(hFile);
    } else {
        xha_printf("The file [%s] does not exist or is not a QDF file\n");
    }
    return iResult;
}
