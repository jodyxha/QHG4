#include <cstdio>
#include <cstring>
#include <cmath>

#include <algorithm>

#include "types.h"
#include "colors.h"
#include "strutils.h"
#include "stdstrutilsT.h"
#include "SequenceProvider.h"
#include "PheneWriter2.h"


bool s_bVerbose = false;

//----------------------------------------------------------------------------
// format accepted
//
bool PheneWriter2::formatAccepted(const std::string sFormat) {
    bool bAccepted = false;
    if (sFormat == FORMAT_BIN) {
        bAccepted = true;
    } else if (sFormat == FORMAT_ASC) {
        bAccepted = true;
    } else {
        bAccepted = false;
    }
    return bAccepted;
}


//----------------------------------------------------------------------------
// writeSequence
//  write ID, mom id, dad id, gender and phenome
//  with sub headers for different locations
//
int PheneWriter2::writeSequence(const std::string sFormat, 
                                SequenceProvider<float> *pGP, 
                                const std::string sOutputFile,  
                                const loc_data &mLocData,  
                                const IDSample *pSample, 
                                bool bFull, bool bHeaders, int iPloidy) {
    int iResult = 0;

    if (sOutputFile != "") {

        
        if (sFormat == FORMAT_BIN) {
            if (s_bVerbose) stdprintf("Writing binary output (genome size %d) to [%s]\n", 
                   pGP->getSequenceSize(), sOutputFile);
            iResult = writePhenesBin(pGP, sOutputFile, mLocData, pSample, bFull, iPloidy);

        } else if (sFormat == FORMAT_ASC) {
            if (s_bVerbose) stdprintf("Writing ascii output (phenome data only; genome size %d) to [%s]\n", 
                   pGP->getSequenceSize(), sOutputFile);
            iResult = writePhenesAsc(pGP, sOutputFile, mLocData, pSample, bFull, bHeaders, iPloidy);  
            

        } else {
            stdfprintf(stderr, "%sUnknown format [%s]%s\n", colors::RED, sFormat, colors::OFF);
            iResult = -1;
        }
    } else {
        stdfprintf(stderr, "%sNo file name given%s\n", colors::RED, colors::OFF);
        iResult = -1;
    }
    return iResult;
}



//----------------------------------------------------------------------------
// writePhenesBin
//   write genome data to binary file.
//   NOTE: the two chromosomes are written on after the other!
//   Format:
//     File       ::= <Header><LocPhenomes>*
//     Header     ::= <Magic><PhenomeSize><NumPhenomes><NumLocs><Ploidy><Full>
//     Magic      ::= "PHNZ"
//     LocGenomes ::= <LocData><PhenomeData>*
//     LocData    ::= <LocNameLen><LocName><LocLon><LocLat><LocDist><Time><NumLocPhenomes>
//     PhenomeData ::= <PhenomeInfo><BinPhenome>
//     PhenomeInfo ::= <ID>[<MomID><DadID><Gender>]<CellID><PheneLon><PheneLat>
//     BinPhenome  ::= <Block>*
//    PhenomeSize    : int
//    NumPhenomes    : int
//    NumLocs        : int
//    Full           : bool
//    LocNameLen     : int  
//    LocName        : char[]    // name of sampling location
//    LocLon         : double    // longitude  of sampling location
//    LocLat         : double    // longitude  of sampling location  
//    LocDist        : double    // sampling radius for sampling location  
//    Time           : double    // sampling time
//    NumLocPhenomes : int       // number of genomes for location
//    ID             : long
//    MomID          : long      // -1 if not specified
//    DadID          : long      // -1 if not specified
//    CellID         : int       //  0 if not specified
//    Gender         : int       //  0: female, 1: male
//    PheneLon       : double    // longitude of genome
//    PheneLat       : double    // latitude of genome
//    Block          : float      // consists of 1- or 2-bit nucleotides
//
int PheneWriter2::writePhenesBin(SequenceProvider<float> *pGP, const std::string sOutputFile, const loc_data &mLocDefs,  const IDSample *pSample, bool bFull, int iPloidy) {
    int iResult = 0;


    FILE *fOut = fopen(sOutputFile.c_str(), "wb");
    if (fOut != NULL) {
        size_t iPhenomeSize = pGP->getSequenceSize();
        int iNumBlocks = iPloidy*iPhenomeSize;
	
        size_t iLen = sizeof(idtype) + 
                      sizeof(int) + 
                      2*sizeof(double) + 
                      iNumBlocks*sizeof(float);
        if (bFull) {
            iLen += 2*sizeof(idtype) + sizeof(int);
        }

        tloc_ids mSelected;
        pSample->getTimeLocationIDSet(mSelected);
        id_agd mIDAD;
        pSample->getIDADMap(mIDAD);

        size_t iNumPhenes = 0;
        tloc_ids::const_iterator it0;
        for (it0 = mSelected.begin(); it0 != mSelected.end(); ++it0) {
            iNumPhenes += it0->second.size();
        }
        size_t iNumLocs = mSelected.size();
        
        int iLenH =  4*sizeof(char)+4*sizeof(int)+sizeof(bool);
        char *pLineH = new char[iLenH];
        // write file header
        char *p0 = pLineH;
        p0 = putMem(p0, "PHNZ", 4*sizeof(char));
        p0 = putMem(p0, &iPhenomeSize, sizeof(int));
        p0 = putMem(p0, &iNumPhenes, sizeof(int));
        p0 = putMem(p0, &iNumLocs, sizeof(int));
        p0 = putMem(p0, &iPloidy, sizeof(int));
        p0 = putMem(p0, &bFull, sizeof(bool));
        size_t iWritten = fwrite(pLineH, iLenH, 1, fOut);
        delete[] pLineH;
        if (iWritten == 1) {

            char *pLine = new char[iLen];
        
            char *pSpecial = NULL;
            size_t iSpecial = 0;
            
            tloc_ids::const_iterator it;
            for (it = mSelected.begin(); (iResult == 0) && (it != mSelected.end()); ++it) {
                const locitem &li = mLocDefs.at(it->first.first);
                double dLon = li.dLon;//*180/M_PI;
                double dLat = li.dLat;//*180/M_PI;
                // write the location header
                size_t iNumPhenomes = it->second.size();

                const char *pName = it->first.first.c_str();
                size_t iNameLen = 1+it->first.first.length();

                size_t iFullLength = iNameLen+2*sizeof(int)+4*sizeof(double);
                if (iSpecial < iFullLength) {
                    if (pSpecial != NULL) {
                        delete[] pSpecial;
                    }
                    iSpecial = iFullLength;
                    pSpecial = new char[iSpecial];
                }
                char *p = pSpecial;
                p = putMem(p, &(iNameLen), sizeof(int));
                p = putMem(p, pName, iNameLen);
                p = putMem(p, &(dLon), sizeof(double));
                p = putMem(p, &(dLat), sizeof(double));
                p = putMem(p, &(li.dDist), sizeof(double));
                p = putMem(p, &(it->first.second), sizeof(double));
                p = putMem(p, &(iNumPhenomes), sizeof(int));
                iWritten = fwrite(pSpecial, iFullLength, 1, fOut);

                if (iWritten == 1) {
                
                    // id are sorted because in set
                    idset v2 = it->second;
                                
                    idset::const_iterator its;
                    for (its=it->second.begin(); its != it->second.end(); ++its) {

                        p = pLine;
                        idtype iID = *its;

                        const float *pPhenome = pGP->getSequence(iID);
                        if (pPhenome != NULL) {
                            agdata *pAD = mIDAD[iID];
                            // write agent data
                            double dLonS = pAD->dLon;
                            double dLatS = pAD->dLat;

                            p = putMem(p, &iID, sizeof(idtype));
                            if (bFull) {
                                p = putMem(p, &(pAD->iMomID), sizeof(idtype));
                                p = putMem(p, &(pAD->iDadID), sizeof(idtype));
                                p = putMem(p, &(pAD->iGender), sizeof(int));
                            }
                            p = putMem(p, &(pAD->iCellID), sizeof(int));
                            p = putMem(p, &dLonS, sizeof(double));
                            p = putMem(p, &dLatS, sizeof(double));
                            
                            // write the genome
                            p = putMem(p, pPhenome, iNumBlocks*sizeof(float));
                            

                            iWritten = fwrite(pLine, iLen, 1, fOut);

                            if (iWritten != 1) {
                                stdfprintf(stderr, "%sCouldn't write genome data to [%s]%s\n", colors::RED, sOutputFile, colors::OFF);
                                iResult = -1;
                            }

                        } else {
                            stdfprintf(stderr, "%sBad ID (no genome): %ld%s\n", colors::RED, iID, colors::OFF);
                            iResult = -1;
                        }
                    }
                
                } else {
                    stdfprintf(stderr, "%sCouldn't write genome header to [%s]%s\n", colors::RED, sOutputFile, colors::OFF);
                    iResult = -1;
                }
            }
            delete[] pSpecial;
            delete[] pLine;
        } else {
            stdfprintf(stderr, "%sCouldn't write file header to [%s]%s\n", colors::RED, sOutputFile, colors::OFF);
            iResult = -1;
        }
        
        fclose(fOut);

    } else {
        stdfprintf(stderr, "%sCouldn't open [%s] for writing%s\n", colors::RED, sOutputFile, colors::OFF);
        iResult = -1;
    }


    return iResult;
}


//----------------------------------------------------------------------------
// agdatacomp
//   compare function for sorting a ontainer of agdata*
//
bool agdatacomp(agdata *p1, agdata *p2) {
    return p1->iID < p2->iID;
}
//----------------------------------------------------------------------------
// writePhenesAsc
//   write genome data to binary file.
//   NOTE: the two chromosomes are written on after the other!
//   Format:
//    File          ::= <Info><LocationBlock>*
//    Info          ::= "# GENES " ("FULL" | "RED")
//    LocationBlock ::= <LocHeader><LocData>
//    LocHeader     ::= "# GROUP" <LocName> "("<LocLon>","<LocLat>") d" <LocDist> "T "<Time>
//    LocData       ::= <GeneHeader><GeneData>
//    GeneHeader    ::= <ID> <MoomID> <DadID> <Gender> <CellID> <GeneLon> <GeneLat>
//    GeneData      ::= ( "A" | "C" | "G" | "T" )*
//    LocName       : char[]    // name of sampling location
//    LocLon        : double    // longitude  of sampling location
//    LocLat        : double    // longitude  of sampling location  
//    LocDist       : double    // sampling radius for sampling location  
//    ID            : int
//    MomID         : int
//    DadID         : int
//    CellID        : int
//    Gender        : int
//    GeneLon       : double    // longitude of genome
//    GeneLat       : double    // latitude of genome
//
//
int PheneWriter2::writePhenesAsc(SequenceProvider<float> *pGP, const std::string sOutputFile, const loc_data &mLocDefs,  const IDSample *pSample, bool bFull, bool bHeaders, int iPloidy) {
    int iResult = 0;

    FILE *fOut = fopen(sOutputFile.c_str(), "wt");
    if (fOut != NULL) {
        // check format string for agent entry below
        int iGOffset = 0;
        if (bHeaders) {
            iGOffset = 12+1+2*(9+1)+8+2;
            if (bFull) {
                iGOffset += 3*(9+1);
            }
            stdfprintf(fOut, "# GENES %s G-OFFSET %d\n", bFull?"full":"red",  iGOffset);
        }

        iResult = 0;
        int iPhenomeSize = pGP->getSequenceSize();

        int iNumBlocks = iPloidy*iPhenomeSize;
        
        loc_agd msLocAD;
        pSample->getLocationADSet(msLocAD);
        sampleinfo::const_iterator it_ltd;
        const sampleinfo &mmvAgentData = pSample->getSampleInfo();
        for (it_ltd = mmvAgentData.begin(); it_ltd != mmvAgentData.end(); ++it_ltd) {
            time_vagdata::const_iterator it_td;
            for (it_td = it_ltd->second.begin(); it_td != it_ltd->second.end(); ++it_td) {
                    
                const locitem &li = mLocDefs.at(it_ltd->first);
                // location header
                // fprintf(fOut, "# GROUP %s (%f,%f) d %f T %f\n", it_ltd->first.c_str(), li.dLon*180/M_PI, li.dLat*180/M_PI, li.dDist, it_td->first);
		// location files have coordinates in degrees
                if (bHeaders) {
                    stdfprintf(fOut, "# GROUP %s (%f,%f) d %f T %f\n", it_ltd->first, li.dLon, li.dLat, li.dDist, it_td->first);
                }

                // sort the IDs
                std::vector<agdata*> v2 = it_td->second;
                std::sort(v2.begin(),v2.end(), agdatacomp);
                
                for (uint i = 0; i < v2.size(); ++i) {
                    agdata *pAD = v2[i];
                    idtype iID = pAD->iID;
                    const float *p = pGP->getSequence(iID);
                    if (p != NULL) {
                        // print agent data ...
                        if (bHeaders) {
                            if (bFull) {
                                // changes here must be reflected in the G-Offset-value
                                stdfprintf(fOut, "%12ld %9ld %9ld %9d %9d % 9.4f % 8.4f  ", 
                                        iID, 
                                        pAD->iMomID, pAD->iDadID, pAD->iGender,
                                        pAD->iCellID, pAD->dLon/* *180/M_PI*/, pAD->dLat/* *180/M_PI*/);
                            } else {
                                // changes here must be reflected in the G-Offset-value
                                stdfprintf(fOut, "%12ld %9d % 9.4f % 8.4f  ", 
                                        iID, 
                                        pAD->iCellID, pAD->dLon/**180/M_PI*/, pAD->dLat/* *180/M_PI*/);
                            }
                        }
                        // ... and agent genome
                        for (int iB = 0; iB < iNumBlocks; iB++) {
                            if (iB == iNumBlocks) {
                                stdfprintf(fOut, " ");
                            }
                            if (std::isnan(*p) or std::isinf(*p)) {
                                stdprintf ("ID %ld: element %d is %s\n", iID, iB, std::isnan(*p)?"nan":"inf");
                            }
                            stdfprintf(fOut, "%+.6e ", *p);
                            p++;
                        }
                        stdfprintf(fOut, "\n");
                    } else {
                        stdfprintf(stderr, "%sBad ID (no phenome): %ld%s\n", colors::RED, v2[i]->iID, colors::OFF);
                        iResult = -1;
                    }
                }
            }
        }                        
        fclose(fOut);
    } else {
        stdfprintf(stderr, "%sCouldn't open [%s] for writing%s\n", colors::RED, sOutputFile, colors::OFF);
        iResult = -1;
    }

    return iResult;
}
