#ifndef __SIMULATOR_H__
#define __SIMULATOR_H__

#include <hdf5.h>

#include "SimParams.h"

#include "gzutils.h"
#include "StatusWriter.h"
#include "OccTracker.h"
#include "IDGen.h"


class Simulator : public SimParams {
public:
    Simulator();
    virtual ~Simulator();

    bool isReady();


    int runSimulation(); // prepare(); run();cleanup();
    

    int writeState(const std::string sQDFOut, int iWhat, std::vector<std::pair<std::string, popwrite_flags>> &vSub, int iDumpMode);

    void showInputs();
    void setInterrupt() { m_bInterrupt = true;};

private:

    int m_iCurStep;

    // -- event handling --
    void checkEvents(int iCurStep);
    void checkFinalEvents();
    int processEvent(EventData *pED);
    int handleWriteEvent(const std::string sDesc, int iDumpMode);
    int handleEnvironmentEvent(const std::string sDesc);
    int handleEnvArrayEvent(const std::string sDesc);
    int handleDumpEvent(const std::string sDesc);
    int handlePopEvent(const std::string sDesc);
    int handleCheckEvent(const std::string sDesc);
    int handleCommEvent(const std::string sDesc);
    int handleUserEvent(const std::string sDesc);
    int handleInterpolEvent(const std::string sDesc);
    int handleScrambleEvent(const std::string sDesc);
    int handleCommLine(const std::string sLine);

    void writeResumeConfig(const std::string sConfigOut);

    // -- looping --
    int preLoop(); 
    int runLoop(); 
    int postLoop();

    // -- setting environment arrays --
    int setGeoArray(hid_t hFile, const std::string sArrName);
    int setClimateArray(hid_t hFile, const std::string sArrName);
    int setVegArray(hid_t hFile, const std::string sArrName);


    int m_iLastCommTime;
    bool m_bInterrupt;
    stringvec m_vDumpNames;

    gzUtils *m_pgzCompressor;
    OccTracker *m_pOcc;
};

#endif

