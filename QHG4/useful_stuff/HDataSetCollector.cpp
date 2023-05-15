#include <cstdio>

#include <hdf5.h>
#include <string>
#include <map>

#include "types.h"
#include "QDFUtils.h"
#include "HDataSetCollector.h"

//----------------------------------------------------------------------------
// createInstance
//
HDataSetCollector *HDataSetCollector::createInstance(const std::string sQDF) {
    HDataSetCollector *pHDSC = new HDataSetCollector();
    int iResult = pHDSC->init(sQDF);
    if (iResult != 0) {
        delete pHDSC;
        pHDSC = NULL;
    }
    return pHDSC;
}


//----------------------------------------------------------------------------
// constructor
//
HDataSetCollector::HDataSetCollector() {
}


//----------------------------------------------------------------------------
// destructor
//
HDataSetCollector::~HDataSetCollector() {
}

//----------------------------------------------------------------------------
// group_info
//  callback for H5Literate
//
herr_t group_info(hid_t loc_id, const char *name, const H5L_info_t *linfo, void *opdata) {
    stringvec *pv = (stringvec *)opdata;
    H5O_info1_t oinfo; 
    H5Oget_info_by_name1(loc_id, name, &oinfo, H5P_DEFAULT);
    if (oinfo.type == H5O_TYPE_GROUP) {
        pv->push_back(name);
    }
    return 0;
}


//----------------------------------------------------------------------------
// dataset_info
//  callback for H5Literate
//
herr_t dataset_info(hid_t loc_id, const char *name, const H5L_info_t *linfo, void *opdata) {
    stringvec *pv = (stringvec *)opdata;
    H5O_info1_t oinfo; 
    H5Oget_info_by_name1(loc_id, name, &oinfo, H5P_DEFAULT);
    if (oinfo.type == H5O_TYPE_DATASET) {
        hid_t ds = H5Dopen(loc_id, name, H5P_DEFAULT);
        if (ds >= 0) {
            hid_t ht = H5Dget_type(ds);
            H5T_class_t hc = H5Tget_class(ht);
            if (hc != H5T_COMPOUND) {
                pv->push_back(name);
            }
            H5Tclose(ht);
            H5Dclose(ds);
        }
    }
    return 0;
}


//----------------------------------------------------------------------------
// init
//
int HDataSetCollector::init(const std::string sQDF) {
    int iResult = -1;
    // open qdf file
    hid_t hFile = qdf_openFile(sQDF, "r");

    stringvec vGroupInfo;
    // find all groups 
    herr_t idx = H5Literate(hFile, H5_INDEX_NAME, H5_ITER_INC, NULL, group_info, &vGroupInfo);
    if (idx == 0) {
        // loop through groups
        stringvec vSubs;
        stringvec::const_iterator it;
        for (it = vGroupInfo.begin(); it != vGroupInfo.end(); ++it) {
            hid_t hGroup = qdf_openGroup(hFile, *it); 
            stringvec vGroupInfo2;
            herr_t idx2 = H5Literate(hGroup, H5_INDEX_NAME, H5_ITER_INC, NULL, group_info, &vGroupInfo2);
            if (idx2 == 0) {
                stringvec::const_iterator it2;
                for (it2 = vGroupInfo2.begin(); it2 != vGroupInfo2.end(); ++it2) {
                    vSubs.push_back(*it+"/"+(*it2));
                }        
            }
            qdf_closeGroup(hGroup);
            iResult = 0;
        }
        vGroupInfo.insert(vGroupInfo.end(), vSubs.begin(), vSubs.end());
        if (iResult == 0) {
            iResult = -1;
            
            for (it = vGroupInfo.begin(); it != vGroupInfo.end(); ++it) {
                stringvec vDataSets2;
                hid_t hGroup = qdf_openGroup(hFile, *it); 
                herr_t idx3 = H5Literate(hGroup, H5_INDEX_NAME, H5_ITER_INC, NULL, dataset_info, &vDataSets2);
                if (idx3 == 0) {
                    stringvec::const_iterator it3;
                    for (it3 = vDataSets2.begin(); it3 != vDataSets2.end(); ++it3) {
                        m_vPathNames.push_back(*it+"/"+(*it3));
                    }        
                    
                }
                qdf_closeGroup(hGroup);
                iResult = 0;
            }
        }
    }       
    qdf_closeFile(hFile);
    return iResult;
}


/*
const stringvec &getPseudos(patmap &mPseudoPats) {
    
    std::string sPath;
    std::string sSpecies;
    patmap::const_iterator it;
    for (it = mPseudoPats.begin(); (iType < 0) && (it != mPseudoPats.end()); ++it) {
        std::cmatch m;
        std::regex pseudoPat(it->second);
        // need to chop off the last one
        stringvec::const_iterator it3;
        for (it3 = vDataSets2.begin(); it3 != vDataSets2.end(); ++it3) {
            if(regex_match(*it3, m, pseudoPat)) {
                iType = it->first;
                vPseudos.ush_back("$1/$2");
                sSpecies = m.format("$2");
            }
        }
  



}
*/
