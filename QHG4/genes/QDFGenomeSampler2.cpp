#include <cstdio>
#include <cstring>
#include <cstdlib>
#include <omp.h>

#include "types.h"
#include "strutils.h"
#include "xha_strutilsT.h"
#include "ParamReader.h"
#include "WELL512.h"
#include "WELLUtils.h"
#include "GeneWriter2.h"
#include "QDFGenomeExtractor2.h"
#include "QDFSequenceExtractor.cpp"


const std::string ATTR_GENOME_SIZE  = "Genetics_genome_size";
const std::string ATTR_BITS_PER_NUC = "Genetics_bits_per_nuc";
const std::string DATASET_GENOME    = "Genome";
const std::string DEFAULT_PHRASE    = "just a random phrase 1 2 3!?";

const int OUT_BIN   = 1;
const int OUT_ASC   = 2;
const int OUT_PLINK = 4;
const int OUT_NUM   = 8;


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
    xha_printf("      [--bit-nucs]\n");
    xha_printf("      [--attr-genome-size=<NameAttrGenomeSize>]\n");
    xha_printf("      [--dataset-genome=<NameDSGenome>]\n");
    xha_printf("      [--genomes-per-buf=<NumGenomesPerBuf>]\n");
    xha_printf("      [-c]\n");
    xha_printf("where\n");
    xha_printf("  QDFPopFile   QDF Population file with genome info\n");
    xha_printf("  SpeciesName  Species name ( if omitted, first species will be used)\n");
    xha_printf("  OutputName   Name for output files\n");
    xha_printf("  format               output format; one of \"ped\", \"bin\", \"asc\", and/or \"num\"\n");
    xha_printf("  Locationfile         name of location file (format: see below)\n");
    xha_printf("  QDFGeoFile           QDF grid file \n");
    xha_printf("  phrase               arbitrary sequence of characters to seed random generator (use quotes if pPhrase contains spaces) (default: [%s])\n", DEFAULT_PHRASE);
    xha_printf("  --dense              use if selected genomes dense in all genomes\n");
    xha_printf("  --bit-nucs           use Bit-Nucleotides instead of normal 2-Bit Nucleotides\n");
    xha_printf("  RefLocationfile      name of location file for reference genome (format: see below)\n");
    xha_printf("  NameAttrGenomeSize   name of the genome size attribute in the QDF file (default \"%s\")\n", ATTR_GENOME_SIZE);
    xha_printf("  NameDSGenome         name of the genome data set in the QDF file (default \"%s\")\n", DATASET_GENOME);
    xha_printf("  NumGenomesPerBuf     determines size of buffer: NumGenomesPerBuf*2*NumLongspergenome (default 1000000)\n");
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
        if (vParts[i] == FORMAT_PLINK) {
            iWhat |= OUT_PLINK;
        }  else if (vParts[i] == FORMAT_BIN) {
            iWhat |= OUT_BIN;
        }  else if (vParts[i] == FORMAT_ASC) {
            iWhat |= OUT_ASC;
        }  else if (vParts[i] == FORMAT_NUM) {
            iWhat |= OUT_NUM;
        }

    }

    return iWhat;
}


//----------------------------------------------------------------------------
// writeOutput
//
int writeOutput(QDFGenomeExtractor2 *pQGE, const std::string sOutputBody, int iWhat, bool bRef, bool bBitNucs) {
    int iResult = 0;

    bool bVerbose = false;

    std::string sOutAsc;
    std::string sOutBin;
    std::string sOutPed;
    std::string sOutMap;
    std::string sOutNum;

    int iErr = 0;

    // get locations
    const loc_data    &mLocDefs = pQGE->getLocData();

    // get agent data
    const IDSample *pSample =  NULL;
    if (bRef) {
        pSample = pQGE->getRefSample();
    } else {
        pSample = pQGE->getSample();
    }

    if ((iWhat & OUT_BIN) != 0) {
        // create name
        sOutBin = xha_sprintf("%s.%sbin", sOutputBody, bRef?"ref.":"");
        if (bVerbose) xha_printf("writing %s\n", sOutBin);
        int iResult1 = GeneWriter2::writeSequence(FORMAT_BIN, pQGE, sOutBin, mLocDefs, pSample, false, false, bBitNucs); // false,false: reduced output, (ignored)
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
        int iResult1 = GeneWriter2::writeSequence(FORMAT_ASC, pQGE, sOutAsc, mLocDefs, pSample, false, false, bBitNucs); // false,false: reduced output, no headers
        if (iResult1 != 0) {
            iResult += iResult1;
            iErr |=  OUT_ASC;
        }
        xha_printf("asc file [%s] written\n", sOutAsc);
    }

    if ((iWhat & OUT_NUM) != 0) {
        // create name
        sOutNum = xha_sprintf("%s.%sdat", sOutputBody, bRef?"ref.":"");
        if (bVerbose) xha_printf("writing %s\n", sOutNum);
        int iResult1 = GeneWriter2::writeSequence(FORMAT_NUM, pQGE, sOutNum, mLocDefs, pSample, true, false, bBitNucs); // true,false: add ID as frist col, (ignored)
        if (iResult1 != 0) {
            iResult += iResult1;
            iErr |=  OUT_NUM;
        }
        xha_printf("num file [%s] written\n", sOutNum);
    }

    // write genes to ped file
    if ((iWhat & OUT_PLINK) != 0) {
        // create name
        sOutPed = xha_sprintf("%s.%sped", sOutputBody, bRef?"ref.":"");
        if (bVerbose) xha_printf("writing %s\n", sOutPed);
        int iResult1 = GeneWriter2::writeSequence(FORMAT_PLINK, pQGE, sOutPed, mLocDefs, pSample, false, false, bBitNucs); // false,false (ignorred), (ignored)
        if (iResult1 != 0) {
            iResult += iResult1;
            iErr |=  OUT_PLINK;
        } else {

            xha_printf("ped file [%s] written\n", sOutPed);
            
            // create name for map file
            sOutMap = xha_sprintf("%s.%smap", sOutputBody, bRef?"ref.":"");
            if (bVerbose) xha_printf("writing %s\n", sOutMap);
            // write map file
            iResult1 = GeneUtils::writePlinkMapFile(sOutMap, pQGE->getSequenceSize());
            if (iResult1 != 0) {
                iResult += iResult1;
                iErr |=  OUT_PLINK;
            } else {
                xha_printf("map file [%s] written\n", sOutMap);
            }
        }
    }

    if (iResult != 0) {
        xha_fprintf(stderr, "writing failed for ");
        if ((iErr & OUT_BIN) != 0) {
            xha_fprintf(stderr, "[%s] ", sOutBin);
        } 
        if ((iErr & OUT_ASC) != 0) {
            xha_fprintf(stderr, "[%s] ", sOutAsc);
        } 
        if ((iErr & OUT_PLINK) != 0) {
            xha_fprintf(stderr, "[%s], [%s]", sOutPed, sOutMap);
        } 
        if ((iErr & OUT_NUM) != 0) {
            xha_fprintf(stderr, "[%s] ", sOutNum);
        } 
        xha_fprintf(stderr, "\n");
         
        // we don't care about the return value of remove
        remove(sOutPed.c_str());
        remove(sOutMap.c_str());
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
    char *pAttGen = NULL;
    char *pAttBitsPerNuc = NULL;
    char *pDSGen  = NULL;
    char *pRefLoc = NULL;
    int  iNumGenomesPerBuf = -1;
    bool bQuiet      = false;
    char *pMapFile   = NULL;
    bool bDense      = false;
    bool bCartesian  = false;
    char *pPhrase    = NULL;
    char *pInSamp    = NULL;
    char *pOutSamp   = NULL;
    char *pRange     = NULL;

    ParamReader *pPR = new ParamReader();
    if (pPR != NULL) {
        pPR->setVerbose(true);
        bool bOK = pPR->setOptions(19,
                                   "-i:S!",  &pPopFile,
                                   "-g:S",   &pGeoFile,
                                   "-s:S",   &pSpecies,
                                   "-o:S!",  &pOutputBody,
                                   "-f:S",   &pFormatList,
                                   "--location-file:S!",    &pLocationFile,
                                   "--ref-location:S",      &pRefLoc,
                                   "--attr-genome-size:S",  &pAttGen,
                                   "--dataset-genome:S",    &pDSGen,
                                   "--genomes-per-buf:i",   &iNumGenomesPerBuf,
				   "--write-index-id-map:S", &pMapFile,
                                   "--dense:0", &bDense, 
				   "--attr-bits-per-nuc:S", &pAttBitsPerNuc, 
                                   "-c:0",                  &bCartesian, 
                                   "--seed:S",              &pPhrase,
                                   "--read-samp:S",         &pInSamp,
                                   "--write-samp:S",        &pOutSamp,
                                   "--set-range:S",         &pRange,
                                   "-q:0", &bQuiet);
        if (bOK) {
            iResult = pPR->getParams(iArgC, apArgV);
            if (iResult >= 0) {
                if (!bQuiet) pPR->display();
               
                // set defaults for unset string  params
                char *pAttGen2        = defaultIfNULL(pAttGen,        ATTR_GENOME_SIZE.c_str());
                char *pAttBitsPerNuc2 = defaultIfNULL(pAttBitsPerNuc, ATTR_BITS_PER_NUC.c_str());
                char *pDSGen2         = defaultIfNULL(pDSGen,         DATASET_GENOME.c_str());
                char *pPhrase2        = defaultIfNULL(pPhrase,        DEFAULT_PHRASE.c_str());
                
                iResult = 0;
                QDFGenomeExtractor2 *pQGE = NULL;
                WELL512 *pWELL = WELLUtils::createWELL(pPhrase2);

                // create QDFGenomeExtractor
                if (pWELL != NULL) {
                    if (pGeoFile != NULL) {
                        pQGE = QDFGenomeExtractor2::createInstance(pGeoFile, pPopFile, pSpecies, pAttGen2, pAttBitsPerNuc2, pDSGen2, pWELL, bCartesian);
                    } else {
                        pQGE = QDFGenomeExtractor2::createInstance(pPopFile, pSpecies, pAttGen2, pAttBitsPerNuc, pDSGen2, pWELL, bCartesian);
                    }
                }

                pQGE->setVerbose(!bQuiet);
                bool bHasRef = (pRefLoc != NULL);

                // create selection
                if (iResult == 0) {
                    iResult = pQGE->createSelection(pLocationFile, pRefLoc, bDense, iNumGenomesPerBuf, pInSamp, pOutSamp);
                    if (iResult != 0) {
                        xha_fprintf(stderr, "error creating selection\n");
                    }
                }

                // check and set range if required
                if ((iResult == 0) && (pRange != NULL)) {
                    int iFirst = 0;
                    int iNum   = 0;
                    if (splitRangeString(pRange, &iFirst, &iNum)) {
                        iResult = pQGE->setRange(iFirst, iNum);
                    } else {
                        xha_printf("Bad range string\n");
                        iResult = -1;
                    }
                } else {
                    iResult = 0;
                }

                // show selected ids
                if (iResult == 0) {
                    // here 
                    if (!bQuiet) {
                        xha_printf("selected %d ids:", pQGE->getNumSelected());
                        const idset &sSelected = pQGE->getSelectedIDs();
                        if (false) {
                            for (idset::const_iterator it = sSelected.begin(); it != sSelected.end(); ++it) {
                                xha_printf(" %ld", *it);
                            }
                        }
                        xha_printf("\n");
                    }
                }


                // write required output
                if (iResult == 0) {
                    int iWhat = OUT_BIN;
                    bool bBitNucs = (pQGE->getBitsPerNuc()==1);


                    if (pFormatList != NULL) {
                        iWhat = getFormats(pFormatList);
                    }
                    iResult = writeOutput(pQGE, pOutputBody, iWhat, false, bBitNucs); //false: not a reference
                    if (iResult == 0) {
                        if (pMapFile != NULL) {
                            iResult = writeIndexIDMap(pQGE->getIndexIDMap(), pMapFile);
                        }    
                    } else {
                        xha_fprintf(stderr, "failed writing outputs\n");
                    }
                    
                    if (bHasRef && (iResult == 0)) {
                        iResult = writeOutput(pQGE, pOutputBody, iWhat, true, bBitNucs);
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

                
                // cleanup
                delete[] pPhrase2;
                delete[] pDSGen2;
                delete[] pAttGen2;
                delete[] pAttBitsPerNuc2;
                delete   pQGE;
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
