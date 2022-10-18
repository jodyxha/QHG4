
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
// calcEuclideanDist
//   (haploid)
//
static float calcEuclideanDist(float *p1, float *p2, int iN) {

    float dS = 0;
    for (int i = 0; i < iN; i++) {
        dS += (p1[i]-p2[i])*(p1[i]-p2[i]);
    }
    return sqrt(dS);
}


//----------------------------------------------------------------------------
// calcDistCross
//   (diploid)
static float calcDistCross(float *p1, float *p2, int iN) {

    float d1 = calcEuclideanDist(p1,    p2,    iN) +
               calcEuclideanDist(p1+iN, p2+iN, iN);

    float d2 = calcEuclideanDist(p1+iN, p2,    iN) +
               calcEuclideanDist(p1,    p2+iN, iN);

    return (d1 < d2)?d1:d2;
}

//----------------------------------------------------------------------------
// calcDistAverages
//   (diploid)
//
static float calcDistAverages(float *p1, float *p2, int iN) {

    float dS = 0;
    for (int i = 0; i < iN; i++) {
        float d1 = (p1[i]+p1[i+iN])/2;
        float d2 = (p2[i]+p2[i+iN])/2;
        dS +=  (d1-d2)*(d1-d2);
    }
    return sqrt(dS);
}





//----------------------------------------------------------------------------
// usage
//   
void usage(char *pApp) {
    printf("%s - phenetic distances\n", pApp);
    printf("Usage;\n");
    printf("  %s -g <phenomefile> -o <outputbody> [-n]\n", pApp);
    printf("      [-m <movestatqdf> [-G <gridqdf>] [-r <referencefile>]]\n");
    printf("      [-d <distance-method>]\n");
    printf("where\n");
    printf("  phenomefile      a file created by QDFPhenSampler\n");
    printf("  outputbody       output file name body\n");
    printf("  movestatqdf      qdf file containing move statistics (for geo/gene dist calculations)\n");
    printf("  gridqdf          qdf file containing grid data (for geo/gene dist calculations)\n");
    printf("                   This file must be specified if movstatqdf contains no grid data\n");
    printf("                   the various outputs will append suffices tp it\n");
    printf("  referencefile    file containing one or more reference phenomes, relative to which\n");
    printf("                   the genetic distances are calculated.\n");
    printf("                   If \"auto\", genetic distances are calculated relative to the \n");
    printf("                   sample group closest to the distance origin.\n");
    printf("  distance-method  \"dist-avg\" | \"dist-cross\"\n");
    printf("                   dist-avg:   distance between averages (diploid only)\n");
    printf("                   dist-cross: minimum of parallel and crosswise comparisons (diploid only)\n");
    printf("\n");
    printf("Outputs:\n");
    printf("always:\n");
    char s[512];
    sprintf(s, TEMPLATE_DIST_MAT, "XXX");
    printf("  '%s'  distance matrix\n", s);
    sprintf(s, TEMPLATE_TABLE, "XXX");
    printf("  '%s'  data table\n", s);
    printf("if reference file is given:\n");
    sprintf(s, TEMPLATE_REF_MAT, "XXX");
    printf("  '%s'   distance to reference\n", s);
    sprintf(s, TEMPLATE_GEO_SEQ, "XXX");
    printf("  '%s'    Geo-Phenetic distances\n", s);
    printf("\n");
}

//----------------------------------------------------------------------------
// readPhenomes2
//   try to read given file a s binary
//
BinPheneFile *readPhenomes2(const char *pPheneFile) {
    int iNumPhenomes = -1;
    BinPheneFile *pBP = BinPheneFile::createInstance(pPheneFile);
    if (pBP != NULL) {
        iNumPhenomes = pBP->read();
        if (iNumPhenomes <= 0) {
            delete pBP;
            pBP = NULL;
        }
    }
    return pBP;
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
    char *pReferenceFile  = NULL;
    char *pDistanceMethod = NULL;


    char sGridQDF[128];
    *sGridQDF = '\0';

    ParamReader *pPR = new ParamReader();
    if (pPR != NULL) {
        bool bOK = pPR->setOptions(6,
                                   "-g:S!",   &pPheneFile,
                                   "-o:S!",   &pOutput,
                                   "-m:S",    &pMoveStatQDF,
                                   "-s:S",    &pSpeciesName,
                                   "-G:s",     sGridQDF,
                                   "-r:S",    &pReferenceFile,
                                   "-d:S",    &pDistanceMethod);
        
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

                printf("Reading phenomes from [%s]\n", pPheneFile); fflush(stdout);
                BinPheneFile *pBP = readPhenomes2(pPheneFile);
                if (pBP != NULL) {
                    int iNumPhenes = pBP->getNumPhenomes();
                    int iPhenomeSize = pBP->getPhenomeSize();
                    mvIDs    = pBP->getvIDs();
                    mIDPhen  = pBP->getIDPhen();
                    mIDNodes = pBP->getIDNodes();
                    mIdLocs  = pBP->getIDLocs();

                    tnamed_ids::const_iterator it;
                    int iii = 0;
                    for (it = mvIDs.begin(); it != mvIDs.end(); ++it) {
                        iii += it->second.size();
                    }
                    printf("after init: #idgen: %zd, #vIDs: %d\n", mIDPhen.size(), iii);

                    iResult = 0;
                    int iPloidy = pBP->getPloidy();

                    // determine distance method
                    SequenceDist<float>::calcdist_t fDistCalc = NULL;
                    if (pDistanceMethod != NULL) {
                        if (iPloidy == 2) {
                            // diploid
                            if (strcmp(pDistanceMethod, "dist-avg") == 0) {
                                printf("PheneDist2 using calcDistAverages\n");
                                fDistCalc = calcDistAverages;
                            } else if (strcmp(pDistanceMethod, "dist-cross") == 0) {
                                printf("PheneDist2 using calcDistCross\n");
                                fDistCalc = calcDistCross;
                            } else if (strcmp(pDistanceMethod, "default") == 0) {
                                printf("PheneDist2 using calcDistAverages (default)\n");
                                fDistCalc = calcDistAverages;
                            } else {
                                printf("unknown distance method for diploid phenomes: [%s]\n", pDistanceMethod);
                                iResult = -1;
                            }
                        } else {
                            // haploid
                            if (strcmp(pDistanceMethod, "dist-euclid") == 0) {
                                printf("PheneDist2 using calcEuclideanDist\n");
                                fDistCalc = calcEuclideanDist;
                            } else if (strcmp(pDistanceMethod, "default") == 0) {
                                printf("PheneDist2 using calcEuclideanDist (default)\n");
                                fDistCalc = calcEuclideanDist;
                            } else {
                                printf("unknown distance method for haploid phenomes: [%s]\n", pDistanceMethod);
                                iResult = -1;
                            }
                        }
                    } else {
                        if (iPloidy == 2) {
                            // diploid
                            printf("PheneDist2 using calcDistAverages (default)\n");
                            fDistCalc = calcDistAverages;
                        } else {
                            printf("PheneDist2 using calcEuclideanDist (default)\n");
                            fDistCalc = calcEuclideanDist;
                        }
                    }


                    if (iResult == 0) {
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
                                        printf("writing dist mat\n");
                                        iResult = pSD->createAndWriteDistMat(pOutput);
                                    }
                                } else {
                                    fprintf(stderr, "Couldn't properly preparenodes\n");
                                }
                            } else {
                                // no grid/movestats
                                iResult = pSD->createAndWriteDistMat(pOutput);
                            
                            }
                        } else {
                            fprintf(stderr, "Couldn't create SequenceDist\n");
                        }
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
