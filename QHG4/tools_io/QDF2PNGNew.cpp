#include <cstdio>
#include <cstdlib>
#include <cstring>

#include <map>
#include <vector>
#include <string>

#include <hdf5.h>

#include "types.h"
#include "qhg_consts.h"
#include "xha_strutilsT.h"
#include "ParamReader.h"
#include "SurfaceGrid.h"
#include "QDFImageExtractor.h"
#include "strutils.h"
#include "LineReader.h"


typedef struct desc_entry {
    std::string sText;
    std::string sQDFList;
    std::string sOutPat;
    desc_entry(std::string sText0, std::string sQDFList0, std::string sOutPat0):sText(sText0),sQDFList(sQDFList0),sOutPat(sOutPat0){};
} desc_entry;

typedef  std::vector<desc_entry> desc_list;

//----------------------------------------------------------------------------
// usage
//
void usage(const std::string sApp) {
    xha_printf("%s - export QDF arrays to PNG\n", sApp);
    xha_printf("Usage:\n");
    xha_printf("  %s -g <qdf_grid> -q <qdf_data>[,<qdf_data>]*]  \n", sApp);
    xha_printf("     -s <w>x<h> -o <outpat>  -a <arrayspec>,[<arrayspec>]*\n");
    xha_printf("    [-c <operation>] [-r <lon_roll>] [-w <ww>x<hw>+<ox>+<oy> ] [-t <text>[:<pos>[:<offs>]]] [-v]\n");
    xha_printf("or");
    xha_printf("  %s -g <qdf_grid> -f <batch_file>\n", sApp);
    xha_printf("     -s <w>x<h>  -a <arrayspec>,[<arrayspec>]*\n");
    xha_printf("    [-c <operation>] [-r <lon_roll>] [-t <text>[:<pos>[:<offs>]]] [-v]\n");
    xha_printf("where\n");
    xha_printf("  qdf_grid       QDF containing a grid and geography\n");
    xha_printf("  qdf_data       QDF file containing data to be extracted\n");
    xha_printf("                 which is not contained in <qdf_grid>\n");
    xha_printf("  batch_file     file containing lines of the form:\n");
    xha_printf("                 <timestamp>\":\"<qdf_data>[,<qdf_data>]*\":\"<output>\n");
    xha_printf("  w              width  of output png\n");
    xha_printf("  h              height of output png\n");
    xha_printf("  ww             width  of depicted area in degrees (default 360)\n");
    xha_printf("  hw             height of depicted area in degrees (default 180)\n");
    xha_printf("  ox             longitude offset of depicted area (default 0)\n");
    xha_printf("  oy             latitude offset of depicted area (default -90)\n");
    xha_printf("  outpat         pattern for output pngs: the substring '###' will be replaced with the array name\n");
    xha_printf("  arrayspec      array specification. Format:\n");
    xha_printf("                 array_spec ::= <array_name>[@<index>][|<lookup>]\n");
    xha_printf("                 array_name  :   name of array (s. below)\n");
    xha_printf("                 index       :   index of qdf in which to look (0: qdf_geogrid, 1-N: qdf-data in given order)\n");
    xha_printf("                 lookup      :   info for lookup, with format <lookup_name>[:<data>]* (s. below)\n");
    xha_printf("  operation      compositing operator (currenly only: 'over' (simple alpha compositing)\n");
    xha_printf("  longroll       longitude for smallest x value (rolls image)\n");
    xha_printf("  text           text to be rendered on image\n");
    xha_printf("  pos            position of text in image (default: '%s')\n", DEF_POS);
    xha_printf("                       +----+----+----+\n");
    xha_printf("                       | UL | UC | UR |\n");
    xha_printf("                       +----+----+----+\n");
    xha_printf("                       | CL | CC | CR |\n");
    xha_printf("                       +----+----+----+\n");
    xha_printf("                       | BL | BC | BR |\n");
    xha_printf("                       +----+----+----+\n");
    xha_printf("  offs             text offset from image border (default: %d)\n", DEF_OFFSET); 
    xha_printf("  -v             verbose\n");
    xha_printf("Arraynames:\n");
    xha_printf("  lon       longitude   (Geography::m_adLongitude)\n");
    xha_printf("  lat       latitude    (Geography::m_adLatitude)\n");
    xha_printf("  alt       altitudes   (Geography::m_adAltitude)\n");
    xha_printf("  ice       ice cover   (Geography::m_abIce)\n");
    xha_printf("  water     water       (Geography::m_adWater)\n");
    xha_printf("  coastal   coastal     (Geography::m_abCoastal)\n");
    xha_printf("  temp      temperature (Climate::m_adAnnualMeanTemp)\n");
    xha_printf("  rain      rainfall    (Climate::m_adAnnualRainfall)\n");
    xha_printf("  npp       total npp   (NPPVegetation::m_adTotalANPP)\n");
    xha_printf("  npp_b     base npp    (NPPVegetation::m_adBaseANPP)\n");
    xha_printf("  dist      travel distance (MoveStats::m_adDist)\n");
    xha_printf("  time      travel time     (MoveStats::m_adTime)\n");
    xha_printf("  pop       population count\n");
    xha_printf("  agent     average of an agent variable\n");
    xha_printf("Lookups:\n");
    xha_printf("  rainbow   data: min, max\n");
    xha_printf("  rainbow2  data: min, max\n");
    xha_printf("  geo       data: min, sealevel, max\n");
    xha_printf("  twotone   data: Sepvalue, RGBA1, RGBA2\n");
    xha_printf("  fadeout   data: min,max, RGBAmax\n");
    xha_printf("  fadeto    data: min,max, RGBAmin, RGBAmax\n");
    /*
    xha_printf("Postprocessing:\n");
    xha_printf("For superposition use imagemagick:\n");
    xha_printf("  composite -compose Over onklop_ice.png onklop_alt.png destination.png\n");
    */
    xha_printf("Call example\n");
    xha_printf("%s -g zworld_22.0_kya_256.qdf \\\n", sApp);
    xha_printf("          -q ooa_pop-Sapiens_ooa__010000.qdf,aternative_ice.qdf \\\n");
    xha_printf("          -s 720x360 \\\n");
    xha_printf("          -o onklop_###_024.PNG \\\n");
    xha_printf("          -a 'alt|geo:-6000:0:6000,ice@2|twotone:0.5:#00000000:#FFFFFFFF, \\\n");
    xha_printf("             pop_sapiens|fadeout:0:40:#00FF00FF,agent:neander[PheneticHyb]|rainbow:0:1' \\\n"); 
    xha_printf("          -r -25 \\\n");
    xha_printf("          -c over\n");
}  


//----------------------------------------------------------------------------
// splitQDFs
//
int splitQDFs(const std::string sQDFData, stringvec &vQDFs) {
    int iResult = -1;

    uint iNum = splitString(sQDFData, vQDFs, ",");
    if (iNum > 0) {
        iResult= 0;
    }

    return iResult;
}


//----------------------------------------------------------------------------
// splitDescFile
//
int splitDescFile(const std::string sDescFile,  desc_list &tsl) {
    int iResult = -1;

    LineReader *pLR = LineReader_std::createInstance(sDescFile, "rt");
    if (pLR != NULL) {
        iResult = 0;
        char *pLine = pLR->getNextLine();
        while ((iResult == 0) && (pLine != NULL) && !pLR->isEoF()) {
            stringvec vParts;
            uint iNum = splitString(pLine, vParts, ":", true);
            if (iNum == 3) {
                tsl.push_back(desc_entry(vParts[0], vParts[1], vParts[2])) ;
                pLine = pLR->getNextLine();
            } else {
                xha_printf("Expected third entry (output pat)\n");
                iResult = -1;
            }

            /*
            char *p0 = strtok(pLine, ":");
            if (p0 != NULL) {
                double dTime = 0;
                if (strToNum(p, &dTime)) {
                    p = strtok(NULL, ":");
                    if (p != NULL) {
                        char *p1 = strtok(NULL, ":");
                        if (p1 != NULL) {

                            tsl.push_back(desc_entry(p0, p, p1));
                            pLine = pLR->getNextLine();
                            
                        } else {
                            xha_printf("Expected third entry (output pat)\n");
                            iResult = -1;
                        }
                    } else {
                        xha_printf("Expected second entry (qdf list)\n");
                        iResult = -1;
                    }
                } else {
                    xha_printf("first entry must be a double [%s]\n", p);
                    iResult = -1;
                }

            } else {
                xha_printf("DescFile empty string?\n");
                iResult = -1;
            }
            */
            
        }
        delete pLR;
    } else {
        xha_printf("Couldn't open Descfile [%s]\n", sDescFile);
    }
    return iResult;
}

//----------------------------------------------------------------------------
// splitWindowString
//  expect: <w>x<h>+<ox>+<oy>
int splitWindowString(const std::string sWindowString, double *pdWV, double *pdHV, double *pdOLon, double *pdOLat) {
    int iResult = 0;
    int iRead = sscanf(sWindowString.c_str(), "%lfx%lf+%lf+%lf", pdWV, pdHV, pdOLon, pdOLat);
    if (iRead != 4) {
        iResult = -1;
        *pdWV = dNaN; 
        *pdHV = dNaN; 
        *pdOLon = dNaN; 
        *pdOLat = dNaN; 
    }
    return iResult;
}

//----------------------------------------------------------------------------
// doSingleImage
//
int doSingleImage(const std::string sQDFData, SurfaceGrid *pSG, const std::string sQDFGrid, const std::string sArrayData, img_prop &ip, const std::string sOutPat, const std::string sCompOp, std::string sText, bool bVerbose) {
    int iResult = 0;
    char *pCopy = NULL;
    stringvec vQDFs;
    vQDFs.push_back(sQDFGrid);
    
    if (!sQDFData.empty()) {
        iResult = splitQDFs(sQDFData, vQDFs);
    }
    if (iResult == 0) {
        //        if (bVerbose) {
            xha_printf("QDF files:\n");
            for (uint i = 0; i < vQDFs.size(); ++i) {
                xha_printf("%2u: %s\n", i, vQDFs[i]);
            }
            //        }
        
        QDFImageExtractor *pQIE = QDFImageExtractor::createInstance(pSG, sQDFGrid, vQDFs, sArrayData, ip, bVerbose);
        if (pQIE != NULL) {
            pQIE->extractAll(sOutPat, sCompOp, sText);
            delete pQIE;
        } else {
            xha_printf("Couldn't create QDFImageExtractor\n");
            iResult = -1;
        }
        
        
        
    } else {
        xha_printf("Couldn't split qdf data\n");
        iResult = -1;
    }
    if (pCopy != NULL) {
        delete[] pCopy;
    }
    return iResult;
}

//----------------------------------------------------------------------------
// main
//
int main(int iArgC, char *apArgV[]) {
    int iResult = 0;

    std::string sQDFGrid{""};
    std::string sQDFData{""};
    std::string sSize{""};
    std::string sOutPat{""};
    std::string sArrayData{""};
    std::string sCompOp{""};
    std::string sDescFile{""};
    std::string sWindow{""};
    std::string sText{""};
    //    std::string sWindow{"360x180+0+-90"};
    double dLonRoll    = -180.0;
    //int    iTime       = -1;             
    bool   bVerbose    = false;

    //    std::string sCompOp("");
    ParamReader *pPR = new ParamReader();
    bool bOK = pPR->setOptions(11,  
                               "-g:s!",   &sQDFGrid,
                               "-q:s",    &sQDFData,
                               "-f:s",    &sDescFile,
                               "-s:s!",   &sSize,
                               "-o:s",    &sOutPat,
                               "-a:s!",   &sArrayData,
                               "-c:s",    &sCompOp,
                               "-r:d",    &dLonRoll,
                               "-t:s",    &sText,
                               "-w:s",    &sWindow,
                               "-v:s",    &bVerbose);
    
    if (bOK) {
        iResult = pPR->getParams(iArgC, apArgV);
        if (iResult >= 0) {
            if (bVerbose) {
                pPR->display();
            }
            int iW = -1;
            int iH = -1;
            xha_printf("Verbose: %s\n", bVerbose?"yes":"no");
            bOK = splitSizeString(sSize.c_str(), &iW, &iH);
            if (bOK) {
                if (!sCompOp.empty()) {
                    sCompOp = sCompOp;
                }

                double dWV=dNaN;
                double dHV=dNaN;
                double dOLon=dLonRoll;
                double dOLat=dNaN;
                if (!sWindow.empty()) {
                    iResult = splitWindowString(sWindow, &dWV, &dHV, &dOLon, &dOLat);
                }
                if (iResult == 0) {
                    
                    img_prop ip(iW, iH, dWV, dHV, dOLon, dOLat);

                    desc_list tsl;

                    if (!sDescFile.empty()) {
                        iResult = splitDescFile(sDescFile, tsl);
                    } else {
                        if (!sOutPat.empty()) {
                            if (!sQDFData.empty()) {
                                tsl.push_back(desc_entry(sText, sQDFData, sOutPat));
                            } else {
                                tsl.push_back(desc_entry(sText, sQDFGrid, sOutPat));
                            }
                            
                        } else {
                            xha_printf("OutPat (-o) must be given if no BatchFile (-f) is used\n");
                            iResult = -1;
                        }
                    }

                
                    if (iResult == 0) {
                        SurfaceGrid *pSG = SurfaceGrid::createInstance(sQDFGrid);
                
                        if (pSG != NULL) {
                        
                            for (uint i = 0; (iResult == 0) && (i < tsl.size()); ++i) {
                                xha_printf("List: [%s]\n", tsl[i].sQDFList);
                                iResult = doSingleImage(tsl[i].sQDFList, 
                                                        pSG, 
                                                        sQDFGrid, 
                                                        sArrayData, 
                                                        ip, 
                                                        tsl[i].sOutPat, 
                                                        sCompOp, 
                                                        tsl[i].sText,
                                                        bVerbose);
                            }
                            delete pSG;
                            if (iResult == 0) {
                                xha_printf("+++ success +++\n");
                            } else {
                                xha_printf("--- failed ---\n");
                            }
                        } else {
                            xha_printf("Couldn't create SurfaceGrid\n");
                        }
                    }
                } else {
                    xha_printf("Couldn't split window string\n");
                    iResult = -1;
                }
            } else {
                xha_printf("Couldn't split size string\n");
                iResult = -1;
            }
        } else {
            xha_printf("ParamReader result: %d\n", iResult);
            xha_printf("%s: %s %s\n", pPR->getErrorMessage(iResult),  pPR->getBadArg(), pPR->getBadVal());
            usage(apArgV[0]);
            iResult = -1;
        }
    } else {
        xha_printf("Error in setOptions\n");
    }
    delete pPR;
    return iResult;
}
