#ifndef __PHENEWRITER2_H__
#define __PHENEWRITER2_H__

#include "types.h"
#include "strutils.h"
#include "SequenceProvider.h"
#include "IDSample.h"
#include "IDSampler2.h"

const std::string FORMAT_ASC   = "asc";
const std::string FORMAT_BIN   = "bin";

const std::string CONTENTS_FULL  = "full";
const std::string CONTENTS_RED   = "red";

class PheneWriter2 {

public:
    bool formatAccepted(const std::string sFormat);

    static int writeSequence(const std::string sFormat, SequenceProvider<float> *pGP, const std::string sOutputFile,  const loc_data &mLocDefs, const IDSample *pSample, bool bFull, bool bHeaders, int iPloidy);
protected:
    static int writePhenesBin(SequenceProvider<float> *pGP, const std::string sOutputFile,  const loc_data &mLocDefs, const IDSample *pSample, bool bFull, int iPloidy);
    static int writePhenesAsc(SequenceProvider<float> *pGP, const std::string sOutputFile,  const loc_data &mLocDefs, const IDSample *pSample, bool bFull, bool bHeaders, int iPloidy);

};

#endif
