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
    xha_printf("%s - Extract genome samples from a QDF population file\n", pApp);
    xha_printf("Usage:\n");
    xha_printf("%s -i <QDFPopFile> [-s <SpeciesName>] -o <OutputName> [-f <format>(\":\"<format>)*]\n", pApp);
    xha_printf("      --location-file=<LocationFile>\n");
    xha_printf("      [-g <QDFGeoFile>]\n");
    xha_printf("      [--seed=<phrase>]\n");
    xha_printf("      [--ref-location=<RefLocationFile>\n");
    xha_printf("      [--dense]\n");
    xha_printf("      [--attr-phenome-size=<NameAttrPhenomeSize>]\n");
    xha_printf("      [--attr-ploidy=<NameAttrPloidy>]\n");
    xha_printf("      [--dataset-phenome=<NameDSPhenome>]\n");
    xha_printf("      [--phenomes-per-buf=<NumPhenomesPerBuf>]\n");
    xha_printf("      [-c]\n");
    xha_printf("where\n");
    xha_printf("  QDFPopFile           QDF Population file with genome info\n");
    xha_printf("  SpeciesName          Species name ( if omitted, first species will be used)\n");
    xha_printf("  OutputName           Name body for output files\n");
    xha_printf("  format               output format; one of \"bin\" and/or \"asc\"\n");
    xha_printf("  Locationfile         name of location file (format: see below)\n");
    xha_printf("  QDFGeoFile           QDF grid file \n");
    xha_printf("  --dense              use if selected genomes dense in all genomes\n");
    xha_printf("  RefLocationfile      name of location file for reference genome (format: see below)\n");
    xha_printf("  NameAttrPhenomeSize  name of the phenome size attribute in the QDF file (default \"%s\")\n", ATTR_PHENOME_SIZE);
    xha_printf("  NameAttrPloidy       name of the phenome size attribute in the QDF file (default \"%s\")\n", ATTR_PLOIDY);
    xha_printf("  NameDSPhenome        name of the phenome data set in the QDF file (default \"%s\")\n", DATASET_PHENOME);
    xha_printf("  NumPhenomesPerBuf    determines size of buffer: NumPhenomesPerBuf*2*phenomesize (default 1000000)\n");
    xha_printf("  phrase               arbitrary sequence of characters to seed random generator (use quotes if pPhrase contains spaces) (default: [%s])\n", DEFAULT_PHRASE);
    xha_printf("  -c                   use cartesian instead of spherical distances (default false)\n");
    xha_printf("\n");
    xha_printf("Location File Format\n");
    xha_printf("  LocationFile ::= <LocationLine>*\n");     
    xha_printf("  LocationLine ::= <identifier> <lon> <lat> <dist> <num> NL\n");     
    xha_printf("  identifier   : string (name of location)\n");
    xha_printf("  longitude    : double (longitude in degrees)\n");
    xha_printf("  latitude     : double (latitude in degrees)\n");
    xha_printf("  dist         : double (sample radius in km)\n");
    xha_printf("  num          : int    (number of elements to sample)\n");
    xha_printf("\n");

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
        sOutBin = xha_sprintf("%s.%sbin", sOutputBody, bRef?"ref.":"");
        if (bVerbose) xha_printf("writing %s\n", sOutBin);
        int iResult1 = PheneWriter2::writeSequence(FORMAT_BIN, pQPE, sOutBin, mLocDefs, pSample, false, false, iPloidy); // false,false: reduced output, (ignored)
        if (iResult1 != 0) {
            iResult += iResult1;
            iErr |=  OUT_BIN;
        }
        xha_printf("bin file [%s] written\n", sOutBin);
    }
    if ((iWhat & OUT_ASC) != 0) {
        // create name
        sOutAsc = xha_sprintf("%s.%sasc", sOutputBody, bRef?"ref.":"");
        if (bVerbose) xha_printf("writing %s\n", sOutAsc);
        int iResult1 = PheneWriter2::writeSequence(FORMAT_ASC, pQPE, sOutAsc, mLocDefs, pSample, false, false, iPloidy); // false,false: reduced output, no headers
        if (iResult1 != 0) {
            iResult += iResult1;
            iErr |=  OUT_ASC;
        }
        xha_printf("asc file [%s] written\n", sOutAsc);
    }


    if (iResult != 0) {
        xha_fprintf(stderr, "writing failed for ");
        if ((iErr & OUT_BIN) != 0) {
            xha_fprintf(stderr, "[%s] ", sOutBin);
        } 
        if ((iErr & OUT_ASC) != 0) {
            xha_fprintf(stderr, "[%s] ", sOutAsc);
        } 
        xha_fprintf(stderr, "\n");
         
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
              xha_fprintf(f, "%u %lu\n", it->first, it->second);
	  }
	  fclose(f);
	  iResult = 0;
     } else {
         xha_fprintf(stderr, "Couldn't open file [%s] for writing\n", pMapOut);
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
                        xha_printf("selected %d ids:", pQPE->getNumSelected());
                        const idset &sSelected = pQPE->getSelectedIDs();
                        if (false) {
                            for (idset::const_iterator it = sSelected.begin(); it != sSelected.end(); ++it) {
                                xha_printf(" %ld", *it);
                            }
                        }
                        xha_printf("\n");
                    }
                } else {
                    xha_fprintf(stderr, "error creating selection\n");
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
                        xha_fprintf(stderr, "failed writing outputs\n");
                    }
                    
                    if (bHasRef && (iResult == 0)) {
                        iResult = writeOutput(pQPE, pOutputBody, iWhat, true, iPloidy);
                        if (iResult == 0) {
                            //ok
                        } else {
                            xha_fprintf(stderr, "couldn't create reference selection\n");
                        }
                    }
                }



                if (iResult == 0) {
                    xha_printf("+++ success +++\n");
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
            xha_fprintf(stderr,"Couldn't set ParamReader options\n");
        }
        delete pPR;
    } else {
        xha_fprintf(stderr, "Couldn't create ParamReader\n");
    }

    return iResult;
}
