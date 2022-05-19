#include <cstdio>
#include <cstring>

#include <hdf5.h>

#include "ParamReader.h"
#include "LineReader.h"
#include "strutils.h"
#include "EQsahedron.h"



//-----------------------------------------------------------------------------
// usage
//
void usage(char *pApp) {
    printf("%s - find node-ids for longitude/Latitude\n", pApp);
    printf("usage:\n");
    printf("  %s -s <subdiv> -i <ignfile> [-f <lonlatfile> | -n <lonlatline>] [-o <output> ]\n", pApp);
    printf("where\n");
    printf("  subdiv           EQsahedron subdivision (usually 256)\n");
    printf("  ignfile          name of grid node file for EQSahedron with specified subdivision\n");
    printf("  lonlatfile       a text file with one space-separated longitude latidude pair per line\n");
    printf("  lonlatline       a line of the form\n");
    printf("                       <lon>:<lat>[,<lon>:<lat>]*\n");
    printf("  output           name for output textfile (if omitted, stdout is used)\n");
    printf("\n");
    printf("Exmple:\n");
    printf("  %s eq128.ieq eq128_002.ign clsAltMoverPop.def:AltMoverData.txt clsGeneticPop.def:GeneticData.txt new.qdf\n", pApp);
    printf("\n");
}



//-----------------------------------------------------------------------------
// readFile
//
int readFile(EQsahedron *pEQ, const char *pInputFile, FILE *fOut) {
    int iResult = 0;
    LineReader *pLR = LineReader_std::createInstance(pInputFile, "rt");
    if (pLR != NULL) {
        while ((iResult == 0) && !pLR->isEoF()) {
            char *pLine = pLR->getNextLine(GNL_IGNORE_ALL);
            if (pLine != NULL) {

                double dLon;
                double dLat;
                
                int iNum = sscanf(pLine, "%lf %lf", &dLon, &dLat);
                if (iNum == 2) {
                    gridtype lNode = pEQ->findNode(dLon, dLat);
                    fprintf(fOut, "%lf %lf %d\n", dLon , dLat, lNode);
                } else {
                    printf("Bad line [%s]\n", pLine);
                    iResult = -1;
                }
            }
        }

        delete pLR;
    } else {
        printf("COuldn't open [%s] for reading\n", pInputFile);
        iResult = -1;
    }
    return iResult;
}

//-----------------------------------------------------------------------------
// readString
//
int readString(EQsahedron *pEQ, char *pLine, FILE *fOut) {
    int iResult = 0;
    
    char *pCtx1;
    char *p1 = strtok_r(pLine, ",", &pCtx1);
    while (p1 != NULL) {
        char *p2 = strchr(p1, ':');
        if (p2 != NULL) {
            *p2++ = '\0';
            double dLon;
            double dLat;
            if (strToNum(p1, &dLon)) {
                if (strToNum(p2, &dLat)) {
                    gridtype lNode = pEQ->findNode(dLon, dLat);
                    fprintf(fOut, "%lf %lf %d\n", dLon , dLat, lNode);    
                } else {
                    printf("Not a valid number: [%s]\n", p2);
                    iResult = -1;
                }
            } else {
                printf("Not a valid number: [%s]\n", p1);
                iResult = -1;
            }
        } else {
            printf("expected ':'\n");
            iResult = -1;
        }
        p1 = strtok_r(NULL, ",", &pCtx1);
    }
    return iResult;
}


//-----------------------------------------------------------------------------
// main
//
int main(int iArgC, char *apArgV[]) {
    int iResult = 0;

    ParamReader *pPR = new ParamReader();
    pPR->setVerbose(true);
    int iSubDivs = 0;
    char *sIgnFile   = NULL;
    char *sLonLatLine = NULL;
    char *sLonLatFile = NULL;
    char *sOutputTxt  = NULL;
    bool bOK = pPR->setOptions(5,
                               "-i:S!",     &sIgnFile,
                               "-s:i!",     &iSubDivs,
                               "-n:S",      &sLonLatLine,
                               "-f:S",      &sLonLatFile,
                               "-o:S",     &sOutputTxt);
    if (bOK) {
        iResult = pPR->getParams(iArgC, apArgV);

        
        int iNumCells =0;


        EQsahedron *pEQ = NULL;
        
        if (iSubDivs >= 0) {
            pEQ = EQsahedron::createInstance(iSubDivs, true);
            if (pEQ != NULL)  {

                iNumCells = EQsahedron::calcNumVerts(iSubDivs);
                printf("Have EQsahedron with %d verts\n", iNumCells);
            }
        } else {
            printf("number of subdivs must be >= 0\n");
        }

        if (pEQ != NULL) {
            FILE *fOut;
            if (sOutputTxt!= NULL) {
                fOut = fopen(sOutputTxt, "w");
            } else {
                fOut = stdout;
            }

            if (sLonLatFile != NULL) {
                iResult = readFile(pEQ, sLonLatFile, fOut);
            } else if (sLonLatLine != NULL) {
                iResult = readString(pEQ, sLonLatLine, fOut);
            } else {
                printf("Need either '-n' or '-f' option\n");
            }

            if (sOutputTxt!= NULL) {
                fclose(fOut);
            }
        }

        delete pEQ;
        delete pPR;
    } else {
        usage(apArgV[0]);
    }

    return iResult;
}
