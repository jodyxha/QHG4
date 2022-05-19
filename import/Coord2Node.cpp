#include <cstdio>
#include <cstring>
#include <vector>
#include <map>

#include <hdf5.h>

#include "strutils.h"
#include "ParamReader.h"
#include "LineReader.h"
#include "Surface.h"
#include "Icosahedron.h"
#include "EQsahedron.h"
#include "Lattice.h"
#include "IcoGridNodes.h"
#include "IcoNode.h"

typedef std::pair<double,double>  coordpair;
typedef std::vector<coordpair>    coordlist;

typedef std::map<int, int>        translation;

//-----------------------------------------------------------------------------
// usage
//
void usage(const char *pApp) {
    printf("%s - find node IDs for coords\n", pApp);
    printf("usage \n");
    printf("  %s -s <surface> -o <outfile> [-t <translation>] (-f <coordfile> | -c <coordstring>)\n", pApp);
    printf("where\n");
    printf("  surface      an .ieq or .ltc file\n"); 
    printf("  outfile      name of outpt file\n"); 
    printf("  coordfile    file consiting of lis of the form\n");
    printf("               <lon>  <lat>\n");
    printf("  translation  file consisting of number pairs\n");
    printf("               <foreignID> <QDFID>\n");
    printf("  coordstring  a string of the form\n");
    printf("               <lon>:<lat>[,<lon>:<lat>]*\n");
    printf("\n");
}

//-----------------------------------------------------------------------------
// getSurface
//   try as Lattice, as IEQ or ICO
//
Surface *getSurface(const char *pSurfFile, int *piNumCells) {
    int iResult = 0;
    Surface *pSurf = NULL;
    *piNumCells = -1;
    Lattice *pLat = new Lattice();
    iResult = pLat->load(pSurfFile);
    if (iResult == 0) {
        pSurf = pLat;
        printf("Have Lattice\n");
        *piNumCells = pLat->getLinkage()->getNumVertices();
    } else {
        EQsahedron *pEQ = EQsahedron::createEmpty();
        iResult = pEQ->load(pSurfFile);
        if (iResult == 0) {
            
            pEQ->relink();
            VertexLinkage *pVL = pEQ->getLinkage();
            printf("VL is %p\n", pVL);

            pSurf = pEQ;
            printf("Have EQsahedron\n");
            *piNumCells = pEQ->getLinkage()->getNumVertices();
        } else {
            Icosahedron *pIco = Icosahedron::create(1, POLY_TYPE_ICO);
            pIco->setStrict(true);
            bool bPreSel = false;
            pIco->setPreSel(bPreSel);
            iResult = pIco->load(pSurfFile);
            if (iResult == 0) {
                printf("Have Icosahedron\n");
                *piNumCells = pIco->getLinkage()->getNumVertices();
            } else {
                pSurf = NULL;
            }
        }
    }
    return pSurf;
}


//----------------------------------------------------------------------------
// fillCoordsFromString
//   expect string of form
//     <lon>:<lat>[,<lon>:<lat>]*
//
int fillCoordsFromString(char *sCoordPair, coordlist &vCoords) {
   int iResult = 0;

   char *p = strtok(sCoordPair, ",");
   while (p != NULL) {
       char *q = strchr(p, ':');
       if (q != NULL) {
           *q = '\0';
           q++;
           double dLon;
           double dLat;
           if (strToNum(p, &dLon)) {
               if (strToNum(q, &dLat)) {
                   vCoords.push_back(coordpair(dLon, dLat));
               } else {
                   iResult = -1;
                   printf("Not a number: [%s]\n", q);
               }
           } else {
               iResult = -1;
               printf("Not a number: [%s]\n", p);
           }
       } else {
           iResult = -1;
           printf("expected ':' in [%s]\n", p);
       }
       p = strtok(NULL, ",");
   }

    return iResult;
}


//----------------------------------------------------------------------------
// fillCoordsFromFile
//
int fillCoordsFromFile(char *sCoordFile, coordlist &vCoords) {
    int iResult = -1;
    LineReader *pLR = LineReader_std::createInstance(sCoordFile, "rt");
    if (pLR != NULL) {
        iResult = 0;

        while ((iResult == 0) && !pLR->isEoF()) {
            char *pLine = pLR->getNextLine(GNL_IGNORE_ALL);
            char * p1 = strtok(pLine, " \t");
            if (p1 != NULL) {
                char *p2 = strtok(NULL, " \t");
                if (p2 != NULL) {
                    double dLon;
                    double dLat;
                    if (strToNum(p1, &dLon)) {
                        if (strToNum(p2, &dLat)) {
                            vCoords.push_back(coordpair(dLon, dLat));
                        } else {
                            iResult = -1;
                            printf("Not a number: [%s]\n", p2);
                        }
                    } else {
                        iResult = -1;
                        printf("Not a number: [%s]\n", p1);
                    }
                } else {
                    printf("Need 2 numbers for a coord pair?\n");
                    iResult = -1;
                }
                
            } else {
                printf("Empty coords?\n");
            }
        }


        delete pLR;
    } else {
        printf("COuldn't open file [%s] for reading\n", sCoordFile);
    }
    return iResult;
}


//-----------------------------------------------------------------------------
// findNodeIDs
//
int findNodeIDs(Surface *pSurf, coordlist &vCoords, translation &mQDF2IDX, FILE *fOut) {
    int iResult = 0;
    bool bTrans = (mQDF2IDX.size() > 0);
    
    for (uint i = 0; (iResult == 0) && (i < vCoords.size()); i++) {
        gridtype lNode = pSurf->findNode(vCoords[i].first, vCoords[i].second);
        if (bTrans) {
            translation::iterator it = mQDF2IDX.find(lNode);
            if (it != mQDF2IDX.end()) {
                lNode = it->second;
            } else {
                printf("value [%d] (%+9.3f,%+9.3f) not found in translation table\n", lNode,vCoords[i].first, vCoords[i].second);
                iResult = -1;
            }
        }
        if (iResult == 0) {
            fprintf(fOut, "%6d  %+9.3f %+9.3f\n", lNode, vCoords[i].first, vCoords[i].second); 
        }
        iResult = 0;
    }
    return iResult;
}


//-----------------------------------------------------------------------------
// readTranslation
//
int readTranslation(char *pIDXFile, translation &mQDF2IDX) {
    int iResult = 0;

    LineReader *pLR = LineReader_std::createInstance(pIDXFile, "rt");
    if (pLR != NULL) {
        while ((iResult == 0) && !pLR->isEoF()) {
            char *p = pLR->getNextLine();
            if (p != NULL) {
                char *p1 = strtok(p, " \t");
                if (p1 != NULL) {
                    char *p2 = strtok(NULL, " \t");
                    if (p2 != NULL) {
                        int iIDX;
                        int iQDF;
                        if (strToNum(p1, &iIDX)) {
                            if (strToNum(p2, &iQDF)) {
                                mQDF2IDX[iQDF] = iIDX;
                            } else {
                                printf("not a number [%s]\n", p2);
                                iResult = -1;
                            }
                        } else {
                            printf("not a number [%s]\n", p1);
                            iResult = -1;
                        }
                    } else {
                        printf("expected two numbers\n");
                        iResult = -1;
                    }
                } else {
                    printf("empty line?");
                    iResult = -1;
                }
            }
                }
    } else {
        printf ("Couldn't open [%s] for reading\n", pIDXFile);
    }
    return  iResult;
}

//-----------------------------------------------------------------------------
// main
//
int main(int iArgC, char *apArgV[]) {
    int iResult = 0;
    char *sSurfFile   = NULL;
    char *sCoordFile  = NULL;
    char *sCoordPair  = NULL;
    char *sOutputFile = NULL;
    char *sTransFile  = NULL;

    ParamReader *pPR = new ParamReader();
    if (pPR != NULL) {
        pPR->setVerbose(true);
        bool bOK = pPR->setOptions(5,
                                   "-s:S!",          &sSurfFile,
                                   "-t:S",           &sTransFile,
                                   "-o:S!",          &sOutputFile,
                                   "-f:S",           &sCoordFile,
                                   "-c:S",           &sCoordPair);
        if (bOK) {
            iResult = pPR->getParams(iArgC, apArgV);
            if (iResult >= 0) {
                coordlist vCoords;
                if (sCoordFile != NULL) {
                    iResult = fillCoordsFromFile(sCoordFile, vCoords);
                } else if (sCoordPair != NULL) {
                    iResult = fillCoordsFromString(sCoordPair, vCoords);
                } else {
                    iResult = -1;
                    usage(apArgV[0]);
                }
                /*
                for (uint i = 0; i < vCoords.size(); i++) {
                    printf("  %f   %f\n", vCoords[i].first, vCoords[i].second);
                }
                */
                if (iResult == 0) {
                    translation mQDF2IDX;
                    if (sTransFile != NULL) {
                        iResult = readTranslation(sTransFile, mQDF2IDX);
                    }
                    int iNumCells = 0;
                    Surface *pSurf = getSurface(sSurfFile, &iNumCells);
                    if (pSurf != NULL) {
                        FILE *fOut = fopen(sOutputFile, "wt");
                        if (fOut != NULL) {
                            findNodeIDs(pSurf, vCoords, mQDF2IDX, fOut);
                            fclose(fOut);
                        } else {
                            printf("Couldn't open [%s] for writing\n", sOutputFile);
                            iResult = -1;
                        }
                        delete pSurf;
                    } else {
                        printf("Couldn't create surface from [%s]\n", sSurfFile);
                        iResult = -1;
                    }
                }
                
            } else {
                usage(apArgV[0]);
            }

        } else {
            printf("Couldn't set ParamReader options\n");
        }
        delete pPR;
    } else {
        printf("Couldn't create ParamReader\n");
    }


    return iResult;

}
