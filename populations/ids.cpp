#include <cstring>
#include <string>
#include <cstdlib>
#include "stdstrutilsT.h"
#include "ids.h"



//--------------------------------------------------------------------------------------
// getName
//
const std::string getName(const NameIVal *nv, int iSize, int iVal) {
    int iHit = -1;
    for (int i = 0; (iHit < 0) && (i < iSize); ++i) {
        if (nv[i].iVal == iVal) {
            iHit = i;
        }
    }
    return std::string((iHit >= 0)? nv[iHit].sName : "");

}


//--------------------------------------------------------------------------------------
// getValue
//  
int getValue(const NameIVal *nv, int iSize, const std::string sName) {
    int iVal = -1;
    
    // is it a number?
    int iT;
    if (strToNum(sName, &iT)) {
        if (!getName(nv, iSize, iT).empty()) {
            iVal = iT;
        }
    } else {

        for (int i = 0; (iVal < 0) && (i < iSize); ++i) {
            // name with or without prefix XXX_
            // if ((strcasecmp(pName, nv[i].pName) == 0) || (strcasecmp(pName, 4+nv[i].pName) == 0)) {
	    // i dont know why we did that "4+nv[i].pName" thing
             if (sName == nv[i].sName) {
                iVal = nv[i].iVal;
            }
        }
    }
    return iVal;
}


//--------------------------------------------------------------------------------------
// spcValue
//  
spcid spcValue(const std::string sSpc) {
    int i =  getValue(s_aSpecies2, sizeof(s_aSpecies2)/sizeof(NameIVal), sSpc);
    
    return (i >= 0)?((spcid) i):(spcid)SPC_NONE;
}


//--------------------------------------------------------------------------------------
// spcName
//
const std::string spcName(spcid iSpc) {
    return getName(s_aSpecies2, sizeof(s_aSpecies2)/sizeof(NameIVal), GET_SPC_SUFF(iSpc));
}

    
//--------------------------------------------------------------------------------------
// clsValue
//  
spcid clsValue(const std::string sCls) {
    int i =  getValue(s_aClasses, sizeof(s_aClasses)/sizeof(NameIVal), sCls);
    return (i >= 0)?((spcid) i):(spcid)CLASS_NONE;
}


//--------------------------------------------------------------------------------------
// clsName
//
const std::string clsName(spcid iCls) {
    return getName(s_aClasses, sizeof(s_aClasses)/sizeof(NameIVal), GET_SPC_SUFF(iCls));
}
    
        
