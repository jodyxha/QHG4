#ifndef __EVENTCHECKER_H__
#define __EVENTCHECKER_H__


class PopLooper;
class EventManager;

// structure to hold event data
typedef struct eventstrings {
    std::string sEventType;
    std::string sEventParams;
    std::string sEventTimes;
    eventstrings(std::string sType, std::string sParams, std::string sTimes) 
        : sEventType(sType),
          sEventParams(sParams),
          sEventTimes(sTimes) {
    };
} eventstrings;

class EventChecker {
public:
    static EventChecker *createInstance(char *pEventDescription, PopLooper *pPopLooper, std::vector<std::string> &vDataDirs, int m_iNumIters, float fStartTime, bool bUpdateEventList);
    static int checkEvents(char *pEventDescription, PopLooper *pPopLooper, std::vector<std::string> &vDataDirs, int m_iNumIters, float fStartTime, bool bUpdateEventList);
    static EventManager *createEventManager(char *pEventDescription, PopLooper *pPopLooper, std::vector<std::string> &vDataDirs, int m_iNumIters, float fStartTime, bool bUpdateEventList);

    ~EventChecker();

    int init(char *pEventDescription);
    EventManager *getEM() {return m_pEM;};
protected:
    std::vector<eventstrings*>   m_vEvents;
    PopLooper                   *m_pPopLooper;
    std::vector<std::string>     m_vDataDirs;
    EventManager                *m_pEM;
    int                          m_iNumIters;
    float                        m_fStartTime;
    bool                         m_bUpdateEventList;

    EventChecker(PopLooper *m_pPopLooper, std::vector<std::string>  &vDataDirs, int iNumIters, float fStartTime, bool bUpdateEventList);
    int readEvents(const std::string sEventDescription);
    int readEventFromString(const std::string sEventString);
    int readEventsFromFile(const std::string sEventFile);

    int checkEvents();
    int checkEvent(const std::string sEventType, const std::string sEventParams);
    int checkWriteParams(const std::string sParams);
    int checkEnvParams(const std::string sParams);
    int checkArrParams(const std::string sParams);
    int checkPopParams(const std::string sParams);
    int checkDumpParams(const std::string sParams);
    int checkInterpolParams(const std::string sParams);
    int checkScrambleParams(const std::string sParams);
    int checkCheckParams(const std::string sParams);
    int checkCommParams(const std::string sParams);
    int checkUserParams(const std::string sParams);

    int checkCommLine(const std::string sLine);
    int checkCommFile(const std::string sFile);

    int addEventToManager(int iEventType, std::string sEventParams, std::string sEventTimes);
    
    bool checkFileExists(const std::string sFile, std::string &sExists);
    bool checkQDFContainsGroup(const std::string sFile, const std::string sGroupName);
    bool checkQDFContainsArr(const std::string sFile, const std::string sGroupName, const std::string sArrName);
};


#endif
