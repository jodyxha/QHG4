#include <cstdio>
#include <cstring>

#include "ParamReader.h"
#include "strutils.h"
#include "xha_strutilsT.h"
#include "HDataSetCollector.h"
#include "QDFDataExtractor.h"


//----------------------------------------------------------------------------
// usage
//
void usage(const char *pApp) {
    xha_printf("%s - extract data from QDF file\n", pApp);
    xha_printf("Usage:\n");
    xha_printf("  %s -g <gridfile> [-d <datafile>] -s<width>x<height>\n", pApp);
    xha_printf("     -p <arrpath> -f <format> [-o <output>]\n");
    xha_printf("where:\n");
    xha_printf("  gridfile     a QDF file containing a grid\n");
    xha_printf("  datafile     a QDF file containing data; if omitted <gridfile> is used\n");
    xha_printf("  arrpath      HDF path to array\n");
    xha_printf("  format       output format; either 'table' or 'list'\n"); 
    xha_printf("  output       name of output txt file; if omitted stdout is used\n");
    xha_printf("\n");
    xha_printf("To find possible values for <arrpath> in a qdf file, do\n");
    xha_printf("  %s -q <QDFfile> \n", pApp);
    xha_printf("If output format 'table' is chosen, the output will  be a matrix with\n");
    xha_printf("<height> rows and <width> columns containing the array data.\n");
    xha_printf("The leftmost column corresponds to -180.0, the rightmost to 180.0\n");
    xha_printf("The first row corresponds to 90.0, the lowest to -90.0\n");
    xha_printf("If output format 'list' is chosen, the output will  be a matrix with\n");
    xha_printf("<height>x<width> rows and 3 columns (<longitude> <latitude> <value>)\n");

    xha_printf("Example:\n");
    xha_printf("  %s -g env_world_256.qdf -s 400x200 -p Geography/Altitude -f table -o poplop.data\n", pApp);
}

//----------------------------------------------------------------------------
// printData
//
int printData(double **pdData, int iW, int iH, char *pOutput) {
    int iResult = 0;
  
    FILE *fOut = NULL;
    if (pOutput != NULL) {
        fOut = fopen(pOutput, "w");
        if (fOut == NULL) {
            xha_printf("Couldn't open [%s[ for writing\n", pOutput);
            iResult = -1;
        }
    } else {
        fOut = stdout;
    }
    
    if (fOut != NULL) {
        if (pdData != NULL) {
            for (int i = 0; i < iH; i++) {
                for (int j = 0; j < iW; j++) {
                    fprintf(fOut, "%7.2f ", pdData[i][j]);
                }
                fprintf(fOut, "\n");
            }
        }
        
        // close file only if not stdout
        if (pOutput != NULL) {
            fclose(fOut);
        } 
    }
    return iResult;
}


//----------------------------------------------------------------------------
// main
//
int main(int iArgC, char *apArgV[]) {
    int iResult = -1;
    
    if (iArgC == 3) {
        if (strcmp(apArgV[1],"-q") == 0) {
            HDataSetCollector *pHDSC = HDataSetCollector::createInstance(apArgV[2]);
            if (pHDSC != NULL) {
                const stringvec vDataSets= pHDSC->getPaths();
                xha_printf("available datasets in [%s]:\n", apArgV[2]);
                stringvec::const_iterator it;
                for (it = vDataSets.begin(); it != vDataSets.end(); ++it) {
                    xha_printf("  %s\n", *it);
                }
                
                const patmap &mPseudoArrPattern = QDFDataExtractor::getPseudoPats();
                
                if (mPseudoArrPattern.size() > 0) {
                    PseudoPopManager *pPPM = PseudoPopManager::createInstance();
                    const stringvec vPseudos = pPPM->findMatches(apArgV[2], apArgV[2]);     
                    for (it = vPseudos.begin(); it != vPseudos.end(); ++it) {
                        xha_printf("  %s\n", *it);
                    }
                    delete pPPM;
                    /*
                      const stringvec vPseudos= pHDSC->getPseudos(mPseudoArrPattern);

                    xha_printf("pseudo arrays in [%s]:\n", apArgV[2]);
                    stringvec::const_iterator it;
                    for (it = vPseudos.begin(); it != vPseudos.end(); ++it) {
                        xha_printf("  %s\n", *it);
                    }
                    */
                }
                
                iResult = 0;
                delete pHDSC;
            }
        }
    }

        
    if (iResult < 0)  {
        ParamReader *pPR = new ParamReader();
        if (pPR != NULL) {
            pPR->setVerbose(true);
            char *sQDFGrid = NULL;
            char *sQDFData = NULL;
            char *sSize    = NULL;
            char *sArrPath = NULL;
            char *sFormat  = NULL;
            char *sOutput  = NULL;
            bool bOK = pPR->setOptions(6,
                                       "-g:S!",     &sQDFGrid,
                                       "-d:S",      &sQDFData,
                                       "-s:S!",     &sSize,
                                       "-p:S!",     &sArrPath,
                                       "-f:S!",     &sFormat,
                                       "-o:S",      &sOutput);
            if (bOK) {
                iResult = pPR->getParams(iArgC, apArgV);
                
                if (iResult >= 0) {
                    char *pQDFData = sQDFGrid;
                    if (sQDFData != NULL) {
                        pQDFData = sQDFData;
                    }
                    
                    int iW;
                    int iH;
                    char sTemp[1024];
                    strcpy(sTemp, sSize);
                    if (splitSizeString(sTemp, & iW, &iH)) {
                        
                        if ((strcmp(sFormat, "table") == 0) || (strcmp(sFormat, "list") == 0)) {
                            iResult = 0;
                            
                            if (iResult == 0) {
                                QDFDataExtractor *pQDE = QDFDataExtractor::createInstance(sQDFGrid, pQDFData, sArrPath, iW, iH);
                                if (pQDE != NULL) {
                                    int iDataW = 0;
                                    int iDataH = 0;
                                    double **pdData = NULL;
                                
                                    if (strcmp(sFormat, "table") == 0) {
                                        pdData = pQDE->extractDataTable();
                                        iDataW = iW;
                                        iDataH = iH;
                                    } else if (strcmp(sFormat, "list") == 0) {
                                        pdData = pQDE->extractDataList();
                                        iDataW = 3;
                                        iDataH = iW*iH;
                                    }
                                
                                    printData(pdData, iDataW, iDataH, sOutput);
                                
                                    // clean up
                                    if (pdData != NULL) {
                                        for (int i = 0; i < iDataH; i++) {
                                            delete[] pdData[i];
                                        }
                                        delete[] pdData;
                                    }
                                }
                            }
                        } else {
                            xha_printf("Bad format [%s] - must be 'table' or 'list'\n", sFormat);
                        }   
                        
                        
                        
                    } else {
                        xha_printf("Not a format string: [%s]\n", sSize);
                    }
                } else {
                    usage(apArgV[0]);
                }
            } else {
                xha_printf("Couldn't set ParamReader options\n");
            }
            delete pPR;
        } else {
            xha_printf("Couldn't create ParamReader\n");
        }
    }
    return iResult;
}
   
