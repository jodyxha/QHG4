#include <cstdio>
#include <cstring>
#include <cstdlib>
#include <omp.h>

#include "types.h"
#include "strutils.h"
#include "ParamReader.h"
#include "WELL512.h"
#include "WELLUtils.h"
#include "PheneWriter2.h"
#include "QDFPhenomeExtractor2.h"
#include "QDFSequenceExtractor.cpp"


#define ATTR_PHENOME_SIZE  "Phenetics_phenome_size"
#define ATTR_PLOIDY        "Phenetics_ploidy"
#define DATASET_PHENOME    "Phenome"
#define DEFAULT_PHRASE    "just a random phrase 1 2 3!?"

#define OUT_BIN   1
#define OUT_ASC   2
#define OUT_PLINK 4
#define OUT_NUM   8


//----------------------------------------------------------------------------
// usage
//
void usage(char *pApp) {
    stdprintf("%s - Extract genome samples from a QDF population file\n", pApp);
    stdprintf("Usage:\n");
    stdprintf("%s -i <QDFPopFile> [-s <SpeciesName>] -o <OutputName> [-f <format>(\":\"<format>)*]\n", pApp);
    stdprintf("      --location-file=<LocationFile>\n");
    stdprintf("      [-g <QDFGeoFile>]\n");
    stdprintf("      [--seed=<phrase>]\n");
    stdprintf("      [--ref-location=<RefLocationFile>\n");
    stdprintf("      [--dense]\n");
    stdprintf("      [--attr-phenome-size=<NameAttrPhenomeSize>]\n");
    stdprintf("      [--attr-ploidy=<NameAttrPloidy>]\n");
    stdprintf("      [--dataset-phenome=<NameDSPhenome>]\n");
    stdprintf("      [--phenomes-per-buf=<NumPhenomesPerBuf>]\n");
    stdprintf("      [-c]\n");
    stdprintf("where\n");
    stdprintf("  QDFPopFile           QDF Population file with genome info\n");
    stdprintf("  SpeciesName          Species name ( if omitted, first species will be used)\n");
    stdprintf("  OutputName           Name body for output files\n");
    stdprintf("  format               output format; one of \"bin\" and/or \"asc\"\n");
    stdprintf("  Locationfile         name of location file (format: see below)\n");
    stdprintf("  QDFGeoFile           QDF grid file \n");
    stdprintf("  --dense              use if selected genomes dense in all genomes\n");
    stdprintf("  RefLocationfile      name of location file for reference genome (format: see below)\n");
    stdprintf("  NameAttrPhenomeSize  name of the phenome size attribute in the QDF file (default \"%s\")\n", ATTR_PHENOME_SIZE);
    stdprintf("  NameAttrPloidy       name of the phenome size attribute in the QDF file (default \"%s\")\n", ATTR_PLOIDY);
    stdprintf("  NameDSPhenome        name of the phenome data set in the QDF file (default \"%s\")\n", DATASET_PHENOME);
    stdprintf("  NumPhenomesPerBuf    determines size of buffer: NumPhenomesPerBuf*2*phenomesize (default 1000000)\n");
    stdprintf("  phrase               arbitrary sequence of characters to seed random generator (use quotes if pPhrase contains spaces) (default: [%s])\n", DEFAULT_PHRASE);
    stdprintf("  -c                   use cartesian instead of spherical distances (default false)\n");
    stdprintf("\n");
    stdprintf("Location File Format\n");
    stdprintf("  LocationFile ::= <LocationLine>*\n");     
    stdprintf("  LocationLine ::= <identifier> <lon> <lat> <dist> <num> NL\n");     
    stdprintf("  identifier   : string (name of location)\n");
    stdprintf("  longitude    : double (longitude in degrees)\n");
    stdprintf("  latitude     : double (latitude in degrees)\n");
    stdprintf("  dist         : double (sample radius in km)\n");
    stdprintf("  num          : int    (number of elements to sample)\n");
    stdprintf("\n");

}

//----------------------------------------------------------------------------
// getFormats
//
int getFormats(std::string sFormatList) {
    int iWhat = 0;

    stringvec vParts;
    uint iNum = splitString(sFormatList, vParts, ":");
    for (uint i = 0; i < iNum; iNum++) {
        if (vParts[i] == FORMAT_BIN) {
            iWhat |= OUT_BIN;
        }  else if (vParts[i] == FORMAT_ASC) {
            iWhat |= OUT_ASC;
        }
    }

    return iWhat;
}


//----------------------------------------------------------------------------
// writeOutput
//
int writeOutput(QDFPhenomeExtractor2 *pQPE, const std::string sOutputBody, int iWhat, bool bRef, int iPloidy) {
    int iResult = 0;

    bool bVerbose = false;

    std::string sOutAsc;
    std::string sOutBin;

    int iErr = 0;

    // get locations
    const loc_data    &mLocDefs = pQPE->getLocData();

    // get agent data
    const IDSample *pSample =  NULL;
    if (bRef) {
        pSample = pQPE->getRefSample();
    } else {
        pSample = pQPE->getSample();
    }

    if ((iWhat & OUT_BIN) != 0) {
        // create name
        sOutBin = stdsprintf("%s.%sbin", sOutputBody, bRef?"ref.":"");
        if (bVerbose) stdprintf("writing %s\n", sOutBin);
        int iResult1 = PheneWriter2::writeSequence(FORMAT_BIN, pQPE, sOutBin, mLocDefs, pSample, false, false, iPloidy); // false,false: reduced output, (ignored)
        if (iResult1 != 0) {
            iResult += iResult1;
            iErr |=  OUT_BIN;
        }
        stdprintf("bin file [%s] written\n", sOutBin);
    }
    if ((iWhat & OUT_ASC) != 0) {
        // create name
        sOutAsc = stdsprintf("%s.%sasc", sOutputBody, bRef?"ref.":"");
        if (bVerbose) stdprintf("writing %s\n", sOutAsc);
        int iResult1 = PheneWriter2::writeSequence(FORMAT_ASC, pQPE, sOutAsc, mLocDefs, pSample, false, false, iPloidy); // false,false: reduced output, no headers
        if (iResult1 != 0) {
            iResult += iResult1;
            iErr |=  OUT_ASC;
        }
        stdprintf("asc file [%s] written\n", sOutAsc);
    }


    if (iResult != 0) {
        stdfprintf(stderr, "writing failed for ");
        if ((iErr & OUT_BIN) != 0) {
            stdfprintf(stderr, "[%s] ", sOutBin);
        } 
        if ((iErr & OUT_ASC) != 0) {
            stdfprintf(stderr, "[%s] ", sOutAsc);
        } 
        stdfprintf(stderr, "\n");
         
    }
    return iResult;
}


//----------------------------------------------------------------------------
// writeIndexIDMap
//
int writeIndexIDMap(const arrpos_ids &mSelected, const char *pMapOut) {
     int iResult = -1;
     FILE *f = fopen(pMapOut, "wt");
     if (f != NULL) {
          arrpos_ids::const_iterator it;
	  for (it = mSelected.begin(); it != mSelected.end(); ++it) {
              stdfprintf(f, "%u %lu\n", it->first, it->second);
	  }
	  fclose(f);
	  iResult = 0;
     } else {
         stdfprintf(stderr, "Couldn't open file [%s] for writing\n", pMapOut);
	 iResult = -1;
     }
     return iResult;
}


//----------------------------------------------------------------------------
// main
//
int main(int iArgC, char *apArgV[]) {
    int iResult = 0;
    char *pPopFile = NULL;
    char *pGeoFile = NULL;
    char *pSpecies = NULL;
    char *pOutputBody = NULL;
    char *pFormatList = NULL;
    char *pLocationFile = NULL;
    char *pAttPhen = NULL;
    char *pAttPloidy = NULL;
    char *pDSPhen  = NULL;
    char *pRefLoc = NULL;
    int  iNumPhenomesPerBuf = -1;
    bool bQuiet      = false;
    char *pMapFile   = NULL;
    bool bDense      = false;
    char *pPhrase    = NULL;
    bool bCartesian  = false;
    char *pInSamp    = NULL;
    char *pOutSamp   = NULL;

    ParamReader *pPR = new ParamReader();
    if (pPR != NULL) {
        pPR->setVerbose(true);
        bool bOK = pPR->setOptions(18,
                                   "-i:S!",  &pPopFile,
                                   "-g:S",   &pGeoFile,
                                   "-s:S",   &pSpecies,
                                   "-o:S!",  &pOutputBody,
                                   "-f:S",   &pFormatList,
                                   "--location-file:S!",   &pLocationFile,
                                   "--ref-location:S",     &pRefLoc,
                                   "--attr-phenome-size:S", &pAttPhen,
                                   "--dataset-phenome:S",   &pDSPhen,
                                   "--phenomes-per-buf:i",  &iNumPhenomesPerBuf,
				   "--write-index-id-map:S", &pMapFile,
                                   "--dense:0", &bDense, 
                                   "--attr-ploidy:S",       &pAttPloidy,
                                   "-c:0", &bCartesian, 
                                   "--seed:S", &pPhrase,
                                   "--read-samp:S",         &pInSamp,
                                   "--write-samp:S",        &pOutSamp,
                                   "-q:0", &bQuiet);
        if (bOK) {
            iResult = pPR->getParams(iArgC, apArgV);
            if (iResult >= 0) {
                if (!bQuiet) pPR->display();

                // set defaults for unset string  params
                char *pAttPhen2   = defaultIfNULL(pAttPhen,   ATTR_PHENOME_SIZE);
                char *pAttPloidy2 = defaultIfNULL(pAttPloidy, ATTR_PLOIDY);
                char *pDSPhen2    = defaultIfNULL(pDSPhen,    DATASET_PHENOME);
                char *pPhrase2    = defaultIfNULL(pPhrase,    DEFAULT_PHRASE);
 
                iResult = -1;
                QDFPhenomeExtractor2 *pQPE = NULL;
                WELL512 *pWELL = WELLUtils::createWELL(pPhrase2);

                if (pWELL != NULL) {
                    if (pGeoFile != NULL) {
                        pQPE = QDFPhenomeExtractor2::createInstance(pGeoFile, pPopFile, pSpecies, pAttPhen2, pAttPloidy2, pDSPhen2, pWELL, bCartesian);
                    } else {
                        pQPE = QDFPhenomeExtractor2::createInstance(pPopFile, pSpecies, pAttPhen2, pAttPloidy2, pDSPhen2, pWELL, bCartesian);
                    }
                }
                bool bHasRef = (pRefLoc != NULL);

                iResult = pQPE->createSelection(pLocationFile, pRefLoc, bDense, iNumPhenomesPerBuf, pInSamp, pOutSamp);
    
                if (iResult == 0) {
                    if (!bQuiet) {
                        stdprintf("selected %d ids:", pQPE->getNumSelected());
                        const idset &sSelected = pQPE->getSelectedIDs();
                        if (false) {
                            for (idset::const_iterator it = sSelected.begin(); it != sSelected.end(); ++it) {
                                stdprintf(" %ld", *it);
                            }
                        }
                        stdprintf("\n");
                    }
                } else {
                    stdfprintf(stderr, "error creating selection\n");
                }


                if (iResult == 0) {
                    int iWhat = OUT_BIN;
                    int iPloidy = pQPE->getPloidy();

                    if (pFormatList != NULL) {
                        iWhat = getFormats(pFormatList);
                    }
                    iWhat |= OUT_ASC;
                    iResult = writeOutput(pQPE, pOutputBody, iWhat, false, iPloidy); //false: not a reference
                    if (iResult == 0) {
                        if (pMapFile != NULL) {
                            iResult = writeIndexIDMap(pQPE->getIndexIDMap(), pMapFile);
                        }    
                    } else {
                        stdfprintf(stderr, "failed writing outputs\n");
                    }
                    
                    if (bHasRef && (iResult == 0)) {
                        iResult = writeOutput(pQPE, pOutputBody, iWhat, true, iPloidy);
                        if (iResult == 0) {
                            //ok
                        } else {
                            stdfprintf(stderr, "couldn't create reference selection\n");
                        }
                    }
                }



                if (iResult == 0) {
                    stdprintf("+++ success +++\n");
                }

               
                delete[] pPhrase2;
                delete[] pDSPhen2;
                delete[] pAttPhen2;
                delete   pQPE;
                delete   pWELL;
                
                
                
            } else {
                usage(apArgV[0]);
            }
        } else {
            stdfprintf(stderr,"Couldn't set ParamReader options\n");
        }
        delete pPR;
    } else {
        stdfprintf(stderr, "Couldn't create ParamReader\n");
    }

    return iResult;
}
