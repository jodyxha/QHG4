#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <csignal>
#include <ctime>


#include "ParamReader.h"
#include "stdstrutils.h"
#include "stdstrutils.cpp"
#include "stdstrutilsT.h"
#include "colors.h"
#include "MessLoggerT.h"
#include "SimLoader.h"
#include "StatusWriter.h"


#define OPT_LOG        "--log-file="

bool s_bCheck;


//-----------------------------------------------------------------------------
// helpParams
//
void helpParams() {
    stdprintf("  -h,                        show help\n");            
    stdprintf("  --help=<topic>             show help for topic (use name of option or \"all\")\n");            
    stdprintf("  --grid=<grid-file>         set grid file\n");     
    stdprintf("  --geo=<geo-file>           set geography file\n");       
    stdprintf("  --climate=<climate-file>   set climate file\n");    
    stdprintf("  --veg=<veg-file>           set vegetation file\n");    
    stdprintf("  --nav=<nav-file>           set navigation file\n");    
    stdprintf("  --pops=<pop-list>          set population files\n");       
    stdprintf("  --output-name=<name>       set output file na,e\n");
    stdprintf("  --data-dirs=<dirnames>     data directories (default: \"./\")\n");    
    stdprintf("  --info-string              information to be written to root group of output files\n");
    stdprintf("  --select=<desc>            string describing which groups t write\n");


}





//----------------------------------------------------------------------------
// main
//
int main(int iArgC, char *apArgV[]) {
    int iResult = 0;

    if (iArgC > 1) {
        std::string sLogFile = DEF_LOG_FILE;
  
        // is a log-file set?
        for (int i = 0; i < iArgC; i++) {
            std::string sCur{apArgV[i]};
            if (sCur.find(OPT_LOG) != std::string::npos) {
                size_t iPos = sCur.find("=");
                if (iPos != std::string::npos) {
                    sLogFile = sCur.substr(iPos+1);
                }
                    
            }

        }
        MessLogger::create(sLogFile);
        time_t t = time(NULL);
        LOG_DISP("-----------------------------------------\n");
        LOG_DISP("Starting Logfile %s\n", trim(ctime(&t)));
        LOG_DISP("-----------------------------------------\n");

        SimLoader *pSSim = new SimLoader();
        iResult = pSSim->readOptions(iArgC, apArgV);

        if (iResult == 0) {
  
            pSSim->showInputs();
            stdprintf("trying to merge---\n");
            int iNumMerged = pSSim->mergePops();
            stdprintf("merged %d pops\n", iNumMerged);
            //iResult = pSim->startLoop();
            if (iResult == 0) {
                
                //iResult = pSSim->writeState(WR_POP);
                iResult = pSSim->write();
                if (iResult == 0) {
                    stdfprintf(stderr, "%s+++success+++%s\n", colors::GREEN, colors::OFF);
                } else {
                    stdfprintf(stderr, "%s---error in write---%s\n", colors::RED, colors::OFF);
                }
            } else {
                stdfprintf(stderr, "%s---error in loops---%s\n", colors::RED, colors::OFF);
            }
            
        } else {
            if (iResult == 3) {
                stdprintf("%susage%s: %d\n", colors::RED, colors::OFF, PARAMREADER_ERR_MANDATORY_MISSING);
                stdprintf("%s", colors::RED);
                SimLoader::helpParams();
                stdprintf("%s\n", colors::OFF);
            } else if (iResult == 2) {
                SimLoader::showTopicHelp(pSSim->getHelpTopic());
            } else {
                stdfprintf(stderr, "%s---error in readOptions---%s\n", colors::RED, colors::OFF);
            }                
        }
        delete pSSim;
       
    } else {
        stdprintf("%susage%s\n", colors::RED, colors::OFF);
        stdprintf("%s", colors::RED);
        SimLoader::helpParams();
        stdprintf("%s\n", colors::OFF);
        
    }
    return iResult;
}
