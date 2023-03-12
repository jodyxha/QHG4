#include <cstdio>
#include <cmath>
#include <filesystem>

#include "strutils.h"
#include "stdstrutilsT.h"

#include "Agent2DataExtractor.h"
#include "AgentDataHistoPie.h"
#include "ParamReader.h"

//----------------------------------------------------------------------------
// usage
//
void usage(const std::string sApp) {
    stdprintf("%s - calculate hstogram foran agent data item and write a pie entry in QDF\n", sApp);
    stdprintf("usage:\n");
    stdprintf("  %s -i <inputQDF> [-o <outputQDF>] -d <DataItemName>\n", sApp);
    stdprintf("     [-p <popName>] -s <samplingInfo> -b <dmin>:<dmax>;<bins>[:!]\n");
    stdprintf("     [-m <mode>[:<mode>]*]\n");
    stdprintf("or\n");
    stdprintf("  %s -i <inputQDF> -l\n", sApp);
    stdprintf("where\n");
    stdprintf("  inputQDF       QDF file containing agent data and a geography group\n");
    stdprintf("  outputQDF      QDF file to which pie group is to be added (if omitted, pie grou will be written to input file)\n");
    stdprintf("  dataItemName   name of data item to extract\n");
    stdprintf("  popName        name of population (if omitzted,  \"sapiens\" is used)\n");
    stdprintf("  samplingInfo   text file containing snmpling info. format see below\n");
    stdprintf("    dmin         minimum value for bins\n");
    stdprintf("    dmax         minimum value for bins\n");
    stdprintf("    bins         number of bins\n");
    stdprintf("  mode           etiher \"txt\", \"csv\" or \"pie\"\n");
    stdprintf("");
    stdprintf("format for sampling info:\n");
    stdprintf("  file ::= <header><NL><datalines>\n");

}

//----------------------------------------------------------------------------
// listDataType
//
int listDataType(std::string sQDFInputFile, std::string sPopName) {
    int iResult = 0;
    std::string sDSPath =  "/Populations/" + sPopName + "/AgentDataSet";
    Agent2DataExtractor *pADE = Agent2DataExtractor::createInstance(sQDFInputFile, sDSPath);
    if (pADE != NULL) {
        stdprintf("Members of compund data type\n");
        pADE->listDataType();
        delete pADE;
    } else  {
        stdprintf("Couldn't create AgentDataExtractor for [%s}", sQDFInputFile);
        iResult = -1;
    }
    return iResult;
}



//----------------------------------------------------------------------------
// main
//
//  -i : qdf input   (where agent data is read from)
//  -o : qdf output  (where pie is written to)
//  -d : name of data item to read
//  -s : list of sampling sites (cell ids + range; if omitted use CellSampling)
//  -b : min_val:max_val:num bins
//  -m : ":"-seprated list of outpu modes ("pie","csv","txt")
//
int main(int iArgC, char *apArgV[]) {
    int iResult = 0;
    
    ParamReader *pPR = new ParamReader();
    if (pPR != NULL) {
        std::string sQDFInputFile  = "";
        std::string sQDFOutputBody = "";
        std::string sPopName       = "sapiens";
        std::string sDataItemName  = "";
        std::string sSamplingInfo  = "";
        std::string sBinInfo       = "";
        std::string sOutputModes   = "pie";
        bool bList    = false;
        bool bVerbose = false;

        bool bText = false;
        bool bCSV  = false;
        bool bPie  = false;
        bool bstd  = false;
        pPR->setVerbose(true);
        bool bOK = pPR->setOptions(9,
                                       "-i:s!",    &sQDFInputFile,
                                       "-o:s",     &sQDFOutputBody,
                                       "-d:s",     &sDataItemName,
                                       "-p:s",     &sPopName,
                                       "-s:s",     &sSamplingInfo,
                                       "-b:s",     &sBinInfo,
                                       "-l:0",     &bList,
                                       "-v:0",     &bVerbose,
                                       "-m:s",     &sOutputModes);
        if (bOK) {
            iResult = pPR->getParams(iArgC, apArgV);
            if (iResult == 0) {
                if (bVerbose) {
                    stdprintf("-i: [%s]\n", sQDFInputFile);
                    stdprintf("-o: [%s]\n", sQDFOutputBody);
                    stdprintf("-d: [%s]\n", sDataItemName);
                    stdprintf("-p: [%s]\n", sPopName);
                    stdprintf("-s: [%s]\n", sSamplingInfo);
                    stdprintf("-l: [%d]\n", bList);
                    stdprintf("-m: [%s]\n", sOutputModes);
                }


                if (!bList) {
                // do sanity check of inputs, and report ptoblrmddomething
                    if ((!sQDFInputFile.empty()) &&
                        (!sDataItemName.empty()) &&
                        (!sSamplingInfo.empty())  &&
                        (!sBinInfo.empty())) {

                        stringvec vParts;
                        uint iNum = splitString(sOutputModes, vParts, ":");
                        if (iNum > 0) {
                            bool bOK = true;
                            for (uint i = 0; bOK && (i < iNum); i++) {
                                if (vParts[i] == "txt") {
                                    bText = true;
                                } else if (vParts[i] == "csv") {
                                    bCSV = true;
                                } else if (vParts[i] == "pie") {
                                    bPie = true;
                                } else if (vParts[i] == "std") {
                                    bstd = true;
                                } else {
                                    stdprintf("unknown output mode [%s]\n", vParts[i]);
                                    bOK = false;
                                }
                            }
                            if (bOK) {
                                iResult = 0;
                            } else {
                                iResult =  -1;
                            }
                        } else if (iNum == 0) {
                            iResult = 0;
                        } else {
                            iResult = -1;
                        }
                    } else {
                        stdprintf("Required options missing;\n");
                                                  
                        if (sDataItemName.empty()) {
                            stdprintf("  -d <data_item>\n");
                        }
                        if (sSamplingInfo.empty()) {
                            stdprintf("  -s <sampling_info>\n");
                        } 
                        if (sBinInfo.empty()) {
                            stdprintf("  -b <bin_info>\n");
                        }
                        usage(apArgV[0]);
                        iResult = -1;
                    }
                }

                // to copy a file: std::filesystem::copy_file
                // now the actual processes
                if (iResult == 0) {
                    if (bList) {
                        listDataType(sQDFInputFile, sPopName);
                    } else {

                        // this will immeidaely extract the required data
                        AgentDataHistoPie *pADHP = AgentDataHistoPie::createInstance(sQDFInputFile, sPopName, sDataItemName);
                        if (pADHP != NULL) {
                            pADHP->setVerbosity(bVerbose);
                            // sample to reduce and group sampling points
                            iResult = pADHP->createSampling(sSamplingInfo);
                            if (iResult == 0) {
                                // make histo data for each sampling group
                                iResult = pADHP->createHisto(sBinInfo);
                                if (iResult == 0) {
                                    if (bPie) {
                                        // write the histogram to pie 
                                        std::string sQDFOutputFile = sQDFOutputBody+".qdf";

                                        if (sQDFOutputBody.empty() || !fileExists(sQDFOutputFile)) {
                                            if (sQDFOutputBody.empty()) {
                                                sQDFOutputBody = sQDFInputFile.substr(0, sQDFInputFile.find(".qdf"))+"_copy";
                                                sQDFOutputFile = sQDFOutputBody+".qdf";
                                            }
                                            std::filesystem::copy_file(sQDFInputFile, sQDFOutputFile);
                                            stdprintf("copied the input file [%s] to [%s]\n", sQDFOutputFile, sQDFOutputFile);
                                        } else {
                                            sQDFOutputFile = sQDFOutputBody+".qdf";
                                        }

                                        iResult = pADHP->writePie(sQDFOutputFile);
                                    }

                                    if (bText && (iResult == 0)) {
                                        iResult = pADHP->writeText(sQDFOutputBody+".txt", bstd);
                                    }
                                    if (bCSV && (iResult == 0)) {
                                        iResult = pADHP->writeCSV(sQDFOutputBody+".csv", bstd);
                                    }
                                    if (iResult == 0) {
                                        stdprintf("+++ success +++\n");
                                    } else {
                                        stdprintf("--- failure ---\n");
                                    } 
                                }
                            }
                            delete pADHP;
                        } else {
                            stdprintf("Coudn't create AgentDataHistoPie\n");
                            iResult = -1;
                        }
                    }
                }

            } else {
                usage(apArgV[0]);
            }

        } else  {
            fprintf(stderr, "Couldn't set ParamReader options\n");
        }
        
        delete pPR;

    } else {
        fprintf(stderr, "Couldn't create ParamReader\n");
    }
    
    return iResult;
} 
