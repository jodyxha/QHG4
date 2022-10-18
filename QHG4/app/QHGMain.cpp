#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <csignal>
#include <ctime>


#include "ParamReader.h"
#include "strutils.h"
#include "colors.h"
#include "Simulator.h"
#include "StatusWriter.h"
#include "PopWriter.h"

#include "MessLoggerT.h"

#define OPT_LOG        "--log-file="
#define OPT_DUMP_INT   "--dump_on_interrupt"

bool s_bCheck;

static Simulator *s_pSim = NULL;


//----------------------------------------------------------------------------
// my_sig
//
void my_sig(int iSig) {
    if (iSig == SIGINT) {
        signal(SIGINT, SIG_DFL);
        s_pSim->setInterrupt();
    }
}


//----------------------------------------------------------------------------
// main
//
int main(int iArgC, char *apArgV[]) {
    int iResult = 0;

    if (iArgC > 1) {
        std::string sLogFile{DEF_LOG_FILE};
        bool bGraceFulHandling = false;

        // is a log-file set?
        for (int i = 0; i < iArgC; i++) {
            if (strstr(apArgV[i], OPT_LOG) == apArgV[i]) {
                char *p = strchr(apArgV[i], '=');
                if (p != NULL) {
                    p++;
                    sLogFile = p;
                }
                    
            } else if (strstr(apArgV[i], OPT_DUMP_INT) == apArgV[i]) {
                bGraceFulHandling = true;
            }

        }
        MessLogger::create(sLogFile);
        time_t t = time(NULL);
        LOG_DISP("-----------------------------------------\n");
        LOG_DISP("Starting Logfile %s\n", trim(ctime(&t)));
        LOG_DISP("-----------------------------------------\n");

        Simulator *pSim = new Simulator();
        iResult = pSim->readOptions(iArgC, apArgV);

        if (iResult == 0) {
            if (bGraceFulHandling) {
            // set new signal handler and save pSim in the static var
                signal(SIGINT, my_sig);
                s_pSim = pSim;
            }


            pSim->showInputs();
            iResult = pSim->runSimulation();
            if (iResult == 0) {
                //            iResult = pSim->writeState(apArgV[3+iOffs], WR_POP);
                if (iResult == 0) {
                    stdprintf("%s+++success+++%s\n", colors::GREEN, colors::OFF);
                }
            } else {
                stdprintf("%serror in loops%s\n", colors::RED, colors::OFF);
            }
            
        } else {
            if (iResult == 3) {
                stdprintf("%susage%s: %d\n", colors::RED, colors::OFF, PARAMREADER_ERR_MANDATORY_MISSING);
                stdprintf("%s", colors::RED);
                Simulator::helpParams();
                stdprintf("%s\n", colors::OFF);
            } else if (iResult == 2) {
                Simulator::showTopicHelp(pSim->getHelpTopic());
            }
        }
        delete pSim;
        MessLogger::free();
       
    } else {
        stdprintf("%susage%s\n", colors::RED, colors::OFF);
        stdprintf("%s", colors::RED);
        Simulator::helpParams();
        stdprintf("%s\n", colors::OFF);
        
    }
    return iResult;
}
