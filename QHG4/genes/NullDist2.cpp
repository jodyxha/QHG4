
#include "ParamReader.h"
#include "LineReader.h"
#include "types.h"
#include "strutils.h"
#include "DistMat.h"
#include "QDFArray.h"
#include "QDFArray.cpp"
#include "BinPheneFile.h"
#include "SequenceDist.h"

#include "DistMat.cpp"
#include "SequenceDist.cpp"

//----------------------------------------------------------------------------
// calcDistDummy
//
static float calcDistDummy(float *p1, float *p2, int iN) {
    return 0.0;
}


//----------------------------------------------------------------------------
// usage
//   
void usage(char *pApp) {
    printf("%s - genetic distances\n", pApp);
    printf("Usage;\n");
    printf("  %s -p <phenomefile> -o <outputbody> [-n]\n", pApp);
    printf("      [-m <movestatqdf> [-G <gridqdf>]\n");
    printf("where\n");
    printf("  genomefile       a file created by GeneSamples\n");
    printf("  outputbody       output file name body\n");
    printf("  movestatqdf      qdf file containing move statistics (for geo/gene dist calculations)\n");
    printf("  gridqdf          qdf file containing grid data (for geo/gene dist calculations)\n");
    printf("                   This file must be specified if movstatqdf contains no grid data\n");
    printf("                   the various outputs will append suffices tp it\n");
    printf("\n");
    printf("Outputs:\n");
    char s[256];
    sprintf(s, TEMPLATE_TABLE, "XXX");
    printf("  '%s'  data table\n", s);
    printf("\n");
}

//----------------------------------------------------------------------------
// readGenomes2
//   try to read given file a s binary
//
BinPheneFile *readPhenomes2(const char *pPheneFile) {
    int iNumPhenomes = -1;
    BinPheneFile *pBG = BinPheneFile::createInstance(pPheneFile);
    if (pBG != NULL) {
        iNumPhenomes = pBG->read();
        if (iNumPhenomes <= 0) {
            delete pBG;
            pBG = NULL;
        }
    }
    return pBG;
}   



//----------------------------------------------------------------------------
// main
//   
int main(int iArgC, char *apArgV[]) {
    int iResult =-1;
    char *pPheneFile      = NULL;
    char *pOutput         = NULL;
    char *pMoveStatQDF    = NULL;
    char *pSpeciesName    = NULL;

    char sGridQDF[128];
    *sGridQDF = '\0';

    ParamReader *pPR = new ParamReader();
    if (pPR != NULL) {
        bool bOK = pPR->setOptions(4,
                                   "-p:S!",   &pPheneFile,
                                   "-o:S!",   &pOutput,
                                   "-m:S",    &pMoveStatQDF,
                                   "-s:S",    &pSpeciesName,
                                   "-G:s",     sGridQDF);
        if (bOK) {
            iResult = pPR->getParams(iArgC, apArgV);
            if (iResult >= 0) {
                if (pSpeciesName == NULL) {
                    pSpeciesName = new char[11];
                    strcpy(pSpeciesName, "dummydummy");
                }
                iResult = -1;
                std::map<idtype,int> mIDNodes;
                tnamed_ids mvIDs;
                id_locs mIdLocs;

                id_phenomes mIDPhen;

                BinPheneFile *pBG = readPhenomes2(pPheneFile);
                if (pBG != NULL) {
                    int iNumPhenes    = pBG->getNumPhenomes();
                    int iPhenomeSize  = pBG->getPhenomeSize();
                    mvIDs    = pBG->getvIDs();
                    mIDPhen  = pBG->getIDPhen();
                    mIDNodes = pBG->getIDNodes();
                    mIdLocs  = pBG->getIDLocs();

                    tnamed_ids::const_iterator it;
                    int iii = 0;
                    for (it = mvIDs.begin(); it != mvIDs.end(); ++it) {
                        iii += it->second.size();
                    }
                    printf("after init: #idgen: %zd, #vIDs: %d\n", mIDPhen.size(), iii);
                    SequenceDist<float>::calcdist_t fDistCalc = calcDistDummy; 
                    SequenceDist<float> *pSD = new SequenceDist<float>(iNumPhenes, iPhenomeSize, mIDNodes, mIDPhen, mvIDs, mIdLocs, fDistCalc);

                    if (pSD != NULL) {
                        // if MoveStat qdf is present, to travel/genetic distance calcs
                        if (pMoveStatQDF != NULL) {
                            // iff no GridFile is specified, assume the MoveStat file 
                            // to contain grid data as well
                            //
                            if (*sGridQDF == '\0') {
                                strcpy(sGridQDF, pMoveStatQDF);
                            }

                            iResult = pSD->prepareNodes(sGridQDF, pMoveStatQDF, pSpeciesName);
                            if (iResult == 0) {
                                iResult = pSD->writeMetaData(pOutput);
                                if (iResult == 0) {
                                } else {
                                    fprintf(stderr, "Couldn't write Metdata\n");
                                }
                            } else {
                                fprintf(stderr, "Couldn't properly preparenodes\n");
                            }
                        }
                    } else {
                        fprintf(stderr, "Couldn't create SequenceDist\n");
                    }
                    
                } else {
                    // nothing to do
                    fprintf(stderr, "Problem reading the phenome file [%s]\n", pPheneFile);
                }
            } else {
                usage(apArgV[0]);
            }
        } else {
            fprintf(stderr, "Couldn't set ParamReader options\n");
        }
        delete pPR;
    } else {
        fprintf(stderr, "Couldn't create ParamReader\n");
    }
    
    if (iResult == 0) {
        printf("+++ success +++\n");
    }

    return iResult;
}
