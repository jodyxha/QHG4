#include <cstdio>
#include <cstdlib>
#include <cstring>

#include <map>
#include <vector>
#include <string>

#include <hdf5.h>

#include "types.h"
#include "qhg_consts.h"
#include "stdstrutilsT.h"
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
    stdprintf("%s - export QDF arrays to PNG\n", sApp);
    stdprintf("Usage:\n");
    stdprintf("  %s -g <qdf_grid> -q <qdf_data>[,<qdf_data>]*]  \n", sApp);
    stdprintf("     -s <w>x<h> -o <outpat>  -a <arrayspec>,[<arrayspec>]*\n");
    stdprintf("    [-c <operation>] [-r <lon_roll>] [-w <ww>x<hw>+<ox>+<oy> ] [-t <text>[:<pos>[:<offs>]]] [-v]\n");
    stdprintf("or");
    stdprintf("  %s -g <qdf_grid> -f <batch_file>\n", sApp);
    stdprintf("     -s <w>x<h>  -a <arrayspec>,[<arrayspec>]*\n");
    stdprintf("    [-c <operation>] [-r <lon_roll>] [-t <text>[:<pos>[:<offs>]]] [-v]\n");
    stdprintf("where\n");
    stdprintf("  qdf_grid       QDF containing a grid and geography\n");
    stdprintf("  qdf_data       QDF file containing data to be extracted\n");
    stdprintf("                 which is not contained in <qdf_grid>\n");
    stdprintf("  batch_file     file containing lines of the form:\n");
    stdprintf("                 <timestamp>\":\"<qdf_data>[,<qdf_data>]*\":\"<output>\n");
    stdprintf("  w              width  of output png\n");
    stdprintf("  h              height of output png\n");
    stdprintf("  ww             width  of depicted area in degrees (default 360)\n");
    stdprintf("  hw             height of depicted area in degrees (default 180)\n");
    stdprintf("  ox             longitude offset of depicted area (default 0)\n");
    stdprintf("  oy             latitude offset of depicted area (default -90)\n");
    stdprintf("  outpat         pattern for output pngs: the substring '###' will be replaced with the array name\n");
    stdprintf("  arrayspec      array specification. Format:\n");
    stdprintf("                 array_spec ::= <array_name>[@<index>][|<lookup>]\n");
    stdprintf("                 array_name  :   name of array (s. below)\n");
    stdprintf("                 index       :   index of qdf in which to look (0: qdf_geogrid, 1-N: qdf-data in given order)\n");
    stdprintf("                 lookup      :   info for lookup, with format <lookup_name>[:<data>]* (s. below)\n");
    stdprintf("  operation      compositing operator (currenly only: 'over' (simple alpha compositing)\n");
    stdprintf("  longroll       longitude for smallest x value (rolls image)\n");
    stdprintf("  text           text to be rendered on image\n");
    stdprintf("  pos            position of text in image (default: '%s')\n", DEF_POS);
    stdprintf("                       +----+----+----+\n");
    stdprintf("                       | UL | UC | UR |\n");
    stdprintf("                       +----+----+----+\n");
    stdprintf("                       | CL | CC | CR |\n");
    stdprintf("                       +----+----+----+\n");
    stdprintf("                       | BL | BC | BR |\n");
    stdprintf("                       +----+----+----+\n");
    stdprintf("  offs             text offset from image border (default: %d)\n", DEF_OFFSET); 
    stdprintf("  -v             verbose\n");
    stdprintf("Arraynames:\n");
    stdprintf("  lon       longitude   (Geography::m_adLongitude)\n");
    stdprintf("  lat       latitude    (Geography::m_adLatitude)\n");
    stdprintf("  alt       altitudes   (Geography::m_adAltitude)\n");
    stdprintf("  ice       ice cover   (Geography::m_abIce)\n");
    stdprintf("  water     water       (Geography::m_adWater)\n");
    stdprintf("  coastal   coastal     (Geography::m_abCoastal)\n");
    stdprintf("  temp      temperature (Climate::m_adAnnualMeanTemp)\n");
    stdprintf("  rain      rainfall    (Climate::m_adAnnualRainfall)\n");
    stdprintf("  npp       total npp   (NPPVegetation::m_adTotalANPP)\n");
    stdprintf("  npp_b     base npp    (NPPVegetation::m_adBaseANPP)\n");
    stdprintf("  dist      travel distance (MoveStats::m_adDist)\n");
    stdprintf("  time      travel time     (MoveStats::m_adTime)\n");
    stdprintf("  pop       population count\n");
    stdprintf("  agent     average of an agent variable\n");
    stdprintf("Lookups:\n");
    stdprintf("  rainbow   data: min, max\n");
    stdprintf("  rainbow2  data: min, max\n");
    stdprintf("  geo       data: min, sealevel, max\n");
    stdprintf("  twotone   data: Sepvalue, RGBA1, RGBA2\n");
    stdprintf("  fadeout   data: min,max, RGBAmax\n");
    stdprintf("  fadeto    data: min,max, RGBAmin, RGBAmax\n");
    /*
    stdprintf("Postprocessing:\n");
    stdprintf("For superposition use imagemagick:\n");
    stdprintf("  composite -compose Over onklop_ice.png onklop_alt.png destination.png\n");
    */
    stdprintf("Call example\n");
    stdprintf("%s -g zworld_22.0_kya_256.qdf \\\n", sApp);
    stdprintf("          -q ooa_pop-Sapiens_ooa__010000.qdf,aternative_ice.qdf \\\n");
    stdprintf("          -s 720x360 \\\n");
    stdprintf("          -o onklop_###_024.PNG \\\n");
    stdprintf("          -a 'alt|geo:-6000:0:6000,ice@2|twotone:0.5:#00000000:#FFFFFFFF, \\\n");
    stdprintf("             pop_sapiens|fadeout:0:40:#00FF00FF,agent:neander[PheneticHyb]|rainbow:0:1' \\\n"); 
    stdprintf("          -r -25 \\\n");
    stdprintf("          -c over\n");
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
                stdprintf("Expected third entry (output pat)\n");
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
                            stdprintf("Expected third entry (output pat)\n");
                            iResult = -1;
                        }
                    } else {
                        stdprintf("Expected second entry (qdf list)\n");
                        iResult = -1;
                    }
                } else {
                    stdprintf("first entry must be a double [%s]\n", p);
                    iResult = -1;
                }

            } else {
                stdprintf("DescFile empty string?\n");
                iResult = -1;
            }
            */
            
        }
        delete pLR;
    } else {
        stdprintf("Couldn't open Descfile [%s]\n", sDescFile);
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
            stdprintf("QDF files:\n");
            for (uint i = 0; i < vQDFs.size(); ++i) {
                stdprintf("%2u: %s\n", i, vQDFs[i]);
            }
            //        }
        
        QDFImageExtractor *pQIE = QDFImageExtractor::createInstance(pSG, sQDFGrid, vQDFs, sArrayData, ip, bVerbose);
        if (pQIE != NULL) {
            pQIE->extractAll(sOutPat, sCompOp, sText);
            delete pQIE;
        } else {
            stdprintf("Couldn't create QDFImageExtractor\n");
            iResult = -1;
        }
        
        
        
    } else {
        stdprintf("Couldn't split qdf data\n");
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
            stdprintf("Verbose: %s\n", bVerbose?"yes":"no");
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
                            stdprintf("OutPat (-o) must be given if no BatchFile (-f) is used\n");
                            iResult = -1;
                        }
                    }

                
                    if (iResult == 0) {
                        SurfaceGrid *pSG = SurfaceGrid::createInstance(sQDFGrid);
                
                        if (pSG != NULL) {
                        
                            for (uint i = 0; (iResult == 0) && (i < tsl.size()); ++i) {
                                stdprintf("List: [%s]\n", tsl[i].sQDFList);
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
                                stdprintf("+++ success +++\n");
                            } else {
                                stdprintf("--- failed ---\n");
                            }
                        } else {
                            stdprintf("Couldn't create SurfaceGrid\n");
                        }
                    }
                } else {
                    stdprintf("Couldn't split window string\n");
                    iResult = -1;
                }
            } else {
                stdprintf("Couldn't split size string\n");
                iResult = -1;
            }
        } else {
            stdprintf("ParamReader result: %d\n", iResult);
            stdprintf("%s: %s %s\n", pPR->getErrorMessage(iResult),  pPR->getBadArg(), pPR->getBadVal());
            usage(apArgV[0]);
            iResult = -1;
        }
    } else {
        stdprintf("Error in setOptions\n");
    }
    delete pPR;
    return iResult;
}
