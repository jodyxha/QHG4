#include <cstdio>

#include "ParamReader.h"
#include "ArrivalChecker.h"

void usage(char *pApp) {
    printf("%s - getting times of closest arrivals to locations<n", pApp);
    printf("Usage:\n");
    printf("  %s -g <QDFGridFile>[:<species>] [-s <QDFStatFile>[:<species>]] -l <LocationFile> [-D <SampDist>] [-c] [-n] [-o] [-C <what>] [-h]\n", pApp);
    printf("or\n");
    printf("  %s l <LocationFile> [-C <what>]\n", pApp);
    printf("where\n");
    printf("  QDFGridFile   a QDF file containing grid and geography\n");
    printf("  QDFStatFile   a QDF file containing move statistics (can be omitted if stats in QDFGridFile)\n");
    printf("  species       name of species to be checked\n"); 
    printf("  LocationFile  a location file\n");
    printf("  SampDist      sampling distance to override the ones specidied in <LocationFile>\n");
    printf("  -c            cartesian distances (for flat grids) (default:  spherical distances) \n");
    printf("  -n            write nice output (default: simple tab-separated text)\n");
    printf("  -o            sort output alphabetically for location names (default: ordering of location file)\n");
    printf("  -C            csv style output of items specified in <what>\n");
    printf("                <what> consists of letters:\n");
    printf("                'l' : location name\n");
    printf("                'c' : coordinates\n");
    printf("                't' : arrival time\n");
    printf("                'n' : agent count\n");
    printf("                'a' : all of the above\n");
    printf("  -h            print a head line before the data line\n");
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

int getItems(const char *pItems) {
    int iItems = ARR_NONE;

    const char *p = pItems;
    while (*p != '\0') {
        switch (*p) {
        case 'l':
            iItems |= ARR_NAME;
            break;
        case 'c':
            iItems |= (ARR_LON | ARR_LAT);
            break;
        case 't':
            iItems |= ARR_TIME;
            break;
        case 'n':
            iItems |= ARR_COUNT;
            break;
        case 'a':
            iItems |= ARR_ALL;
            break;
        }
        p++;
    }
    return iItems;
} 

int main(int iArgC, char *apArgV[]) {
    int iResult = -1;

    ParamReader *pPR = new ParamReader();
    if (pPR != NULL) {
        //        char  *pQDFGrid    = NULL;
        //        char  *pQDFStats   = NULL;
        std::string sQDFGrid  = "";
        std::string sQDFStats = "";
        std::string sLocFile  = "";
        char  *pOutputFile = NULL;
        double dDistance   = 0;
        bool   bCartesian  = false;
        bool   bNice       = false;
        bool   bSort       = false;
        char  *pCSVItems   = NULL;
        bool   bHead       = false;

        bool bOK = pPR->setOptions(10,
                                   "-g:s",    &sQDFGrid,
                                   "-s:s",    &sQDFStats,
                                   "-l:s!",   &sLocFile,
                                   "-D:d",    &dDistance,
                                   "-c:0",    &bCartesian,
                                   "-n:0",    &bNice,
                                   "-o:0",    &bSort,
                                   "-C:S",    &pCSVItems,
                                   "-h:0",    &bHead,
                                   "-f:S",    &pOutputFile);

        if (bOK) {
            iResult = pPR->getParams(iArgC, apArgV);
            if (iResult >= 0) {
                iResult = 0;
                std::string sStats;
                if (sQDFStats == "") {
                    sStats = sQDFGrid;
                } else {
                    sStats = sQDFStats;
                }
                int iItems = ARR_NONE;
                if (pCSVItems != NULL) {
                    iItems = getItems(pCSVItems);
                }

                bool bClose = false;
                FILE *fOut = stdout;
                if (pOutputFile != NULL) {
                    fOut = fopen(pOutputFile, "at");
                    if (fOut != NULL) {
                        bClose = true;
                    } else {
                        fprintf(stderr, "ArrivalCheck couldn't open [%s][ for writing\n", pOutputFile);
                        fprintf(stderr, "Writing output to stdout\n");
                        fOut = stdout;
                    }
                }
                        

                ArrivalChecker *pAC = ArrivalChecker::createInstance(sQDFGrid, sStats, sLocFile, dDistance, fOut);
                if (pAC != NULL) {
                    if (sQDFGrid != "") {
                        pAC->findClosestCandidates(!bCartesian);
                    }
                    if (pCSVItems != NULL) {
                        pAC->showCSV(bHead, iItems);
                    } else {
                        pAC->showTable(bNice, bSort);
                    }
                    delete pAC;
                    fprintf(stderr, "+++ success for [%s] +++\n", sStats.c_str());
               } else {
                    fprintf(stderr, "Couldn't create ArrivalChecker\n");
                    iResult = -1;
                }
                
                if (bClose) {
                    fclose(fOut);
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

