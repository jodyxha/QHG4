#ifndef __GENEWRITER2_H__
#define __GENEWRITER2_H__

#include <string>
#include "SequenceProvider.h"
#include "IDSample.h"
#include "IDSampler2.h"


const std::string FORMAT_PLINK = "plink";
const std::string FORMAT_ASC   = "asc";
const std::string FORMAT_BIN   = "bin";
const std::string FORMAT_NUM   = "num";

const std::string CONTENTS_FULL  = "full";
const std::string CONTENTS_RED   = "red";

class GeneWriter2 {

public:
    static bool formatAccepted(const std::string sFormat);

    static int writeSequence(const std::string sFormat, SequenceProvider<ulong> *pGP, const std::string sOutputFile,  const loc_data &mLocDefs,  const IDSample *pSample, bool bFull, bool bHeaders, bool bBitNucs);
 
protected:
 
    static int writeGenesPlink(SequenceProvider<ulong> *pGP, const std::string sOutputFile,  const loc_data &mLocDefs, const IDSample *pSample);
    static int writeGenesAsc(SequenceProvider<ulong> *pGP, const std::string sOutputFile,  const loc_data &mLocDefs, const IDSample *pSample, bool bFull, bool bHeaders, bool bBitNucs);
    static int writeGenesBin(SequenceProvider<ulong> *pGP, const std::string sOutputFile,  const loc_data &mLocDefs, const IDSample *pSample, bool bFull, bool bBitNucs);
    static int writeGenesNum(SequenceProvider<ulong> *pGP, const std::string sOutputFile, const IDSample *pSample, bool bFull, bool bBitNucs);

};



#endif

