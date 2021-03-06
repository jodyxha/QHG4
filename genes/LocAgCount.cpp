#include <cstdio>

#include "ParamReader.h"
#include "LocAgCounter.h"

void usage(char *pApp) {
    printf("%s - getting times of closest arrivals to locations<n", pApp);
    printf("Usage:\n");
    printf("  %s -g <QDFGridFile> [-s <QDFStatFile>] -l <LocationFile> [-c] [-n] [-o]\n", pApp);
    printf("where\n");
    printf("  QDFGridFile   a QDF file containing grid and geography\n");
    printf("  QDFStatFile   a QDF file containing move statistics (can be omitted if stats in QDFGridFile)\n");
    printf("  LocationFile  a location file\n");
    printf("  SampDist      sampling distance to override the ones specidied in <LocationFile>\n");
    printf("  -c            cartesian distances (for flat grids) (default:  spherical distances) \n");
    printf("  -n            write nice output (default: simple tab-separated text)\n");
    printf("  -o            sort output alphabetically for location names (default: ordering of location file)\n");
    printf("Format of a location file:\n");
    printf("  locfile ::= <locline>*\n");
    printf("  locline ::= <locname> <lon> <lat> <dist> <num>\n");
    printf("  locname :  name of location (string)\n");
    printf("  lon     :  longitude of location (number)\n");
    printf("  lat     :  latitude of location (number)\n");
    printf("  radius  :  search radius around location (number)\n");
    printf("  num     :  number of individuals to sample (not used here)\n");
    printf("\n");
}


int main(int iArgC, char *apArgV[]) {
    int iResult = -1;

    ParamReader *pPR = new ParamReader();
    if (pPR != NULL) {
        char  *pQDFGrid   = NULL;
        char  *pQDFStats  = NULL;
        char  *pLocFile   = NULL;
        bool   bCartesian = false;
        bool   bNice      = false;
        bool   bSort      = false;
   
        bool bOK = pPR->setOptions(6,
                                   "-g:S!",   &pQDFGrid,
                                   "-s:S",    &pQDFStats,
                                   "-l:S!",   &pLocFile,
                                   "-c:0",    &bCartesian,
                                   "-n:0",    &bNice,
                                   "-o:0",    &bSort);

        if (bOK) {
            iResult = pPR->getParams(iArgC, apArgV);
            if (iResult >= 0) {
                char *pStats;
                if (pQDFStats == NULL) {
                    pStats = pQDFGrid;
                } else {
                    pStats = pQDFStats;
                }
                LocAgCounter *pLAC = LocAgCounter::createInstance(pQDFGrid, pStats, pLocFile, bCartesian);
                if (pLAC != NULL) {
                    pLAC->doCounts();
                    pLAC->show(bNice, bSort);
                    delete pLAC;
                } else {
                    fprintf(stderr, "Couldn't create LocAgCounter\n");
                    iResult = -1;
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
    return iResult;
}
