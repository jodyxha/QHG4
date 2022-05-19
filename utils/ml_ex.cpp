#include "MessLoggerT.h"


void logBlock(std::string sMode) {
    LOG_STATUS("a normal status message (%s)\n", sMode);
    LOG_WARNING("a normal warning message (%s)\n", sMode);
    LOG_ERROR("a normal error message (%s)\n", sMode);
    LOG_DISP("a normal disp message (%s)\n", sMode);
    
    LOG_STATUS2("a dual status message (%s)\n", sMode);
    LOG_WARNING2("a dual warning message (%s)\n", sMode);
    LOG_ERROR2("a dual error message (%s)\n", sMode);
    LOG_DISP2("a dual disp message (%s)\n", sMode);

}

int main(int iArgC, char *apArgV[]) {
    int iResult = 0;
    
    MessLogger::create("glubber.log");
    MessLogger::usePrefix(false);
    MessLogger::useColors(false);

    logBlock("prefix:off, color:off");
    MessLogger::useColors(true);
    logBlock("prefix:off, color:on");
    MessLogger::usePrefix(true);
    logBlock("prefix:on, color_on");
    MessLogger::useColors(false);
    logBlock("prefix:on, color:off");
    MessLogger::usePrefix(false);
    logBlock("prefix:off, color:off");

    stdprintf("so far:\n");
    stdprintf("  %d status messages\n",  MessLogger::getNumStatus());
    stdprintf("  %d warning messages\n", MessLogger::getNumWarning());
    stdprintf("  %d error messages\n",   MessLogger::getNumError());
    stdprintf("  %d disp messages\n",    MessLogger::getNumDisp());

    stdprintf("\n");
    stdprintf("The contents of the log file so far\n");
    
    MessLogger::usePrefix(true);
    MessLogger::showLog(MessLogger::SHOW_WARNING | MessLogger::SHOW_ERROR);

    MessLogger::free();
    stdprintf("just checking...\n");
    return iResult;
}
 
