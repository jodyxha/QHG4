#ifndef __POPBASE_H__
#define __POPBASE_H__

#include <hdf5.h>

#include <set>
#include <string>
#include "types.h"
#include "Observable.h"
#include "WELL512.h"
#include "SCellGrid.h"

class ParamProvider2;

class LineReader;
class PopBase : public Observable {
public:
    virtual ~PopBase() {};
    
    virtual int  setPrioList() = 0;
    virtual uint getPrios(std::set<uint> &vPrios)=0;
    virtual int  removeAction(std::string name)=0;

    virtual int preLoop()=0;
    virtual int postLoop()=0;
    virtual int doActions(uint iPrio, float fTime)=0;
    virtual int initializeStep(float fTime)=0;
    virtual int finalizeStep()=0;
    virtual int preWrite(float fTime)=0;

    virtual void registerMove(int iCellIndexFrom, int iAgentIndex, int iCellIndexTo)=0;
    virtual void registerBirth(int iCellIndex, int iMotherIndex, int iFatherIndex=-1)=0;
    virtual void registerDeath(int iCellIndex, int iAgentIndex)=0;
    virtual int  reserveAgentSpace(int iNumAgents)=0;

    virtual void  showAgents()=0;
    virtual ulong getNumAgents(int iCellIndex)=0;
    virtual ulong getNumAgentsTotal()=0;
    virtual ulong getNumAgentsEffective()=0;
    virtual ulong getNumAgentsMax()=0;
    //@@ to be removed    virtual ulong getNumEvents(int iEventMask)=0;
    virtual void  updateTotal()=0;  
    virtual void  updateNumAgentsPerCell()=0;


    virtual int readSpeciesData(ParamProvider2 *pPP)=0;
    virtual const std::string getClassName()=0;
    virtual const std::string getSpeciesName()=0;
    virtual size_t agentDataSize() const = 0; 
    virtual int addAgent(int iCellIndex, char *pData, bool bUpdateCOunts)=0;
    virtual int addAgentData(int iCellIndex, int iAgentIndex, char **ppData)=0;
    virtual int createAgentAtIndex(int iAgentIndex, int iCellIndex)=0;
    virtual void setAgentBasic(int iAgentIndex, void *pBasic)=0;
    virtual int getFreeIndex()=0;
    virtual size_t agentRealSizeQDF() const = 0; 

    virtual void setAgentDataType()=0;
    virtual hid_t createAgentDataTypeQDF()=0;
    virtual int writeSpeciesDataQDF(hid_t hSpeciesGroup)=0;
    virtual int readSpeciesDataQDF(hid_t hSpeciesGroup)=0;
    virtual int writeAgentDataQDF(hid_t hDataSpace, hid_t hDataSet, hid_t hAgentType)=0;
    virtual int readAgentDataQDF(hid_t hDataSpace, hid_t hDataSet, hid_t hAgentType)=0;
    virtual int writeAdditionalDataQDF(hid_t hSpeciesGroup)=0;
    virtual int readAdditionalDataQDF(hid_t hSpeciesGroup)=0;

    virtual int dumpSpeciesDataQDF(hid_t hSpeciesGroup, int iDumpMode)=0;
    virtual int restoreSpeciesDataQDF(hid_t hSpeciesGroup)=0;
    virtual int dumpAgentDataQDF(hid_t hDataSpace, hid_t hDataSet, hid_t hAgentType)=0;
    virtual int restoreAgentDataQDF(hid_t hDataSpace, hid_t hDataSet, hid_t hAgentType)=0;
    virtual int dumpAdditionalDataQDF(hid_t hSpeciesGroup)=0;
    virtual int restoreAdditionalDataQDF(hid_t hSpeciesGroup)=0;

    virtual int mergePop(PopBase *pPop)=0;
    virtual int modifyAttributes(const std::string sAttrName, double dValue)=0;
    
    virtual idtype getMaxLoadedID()=0;
    virtual idtype getUID()=0; 
    virtual void compactData()=0;
    virtual int  checkLists()=0;

    virtual int  getNumCells()=0;

    virtual void randomize(int i)=0;

    virtual int   getFirstAgentIndex()=0;
    virtual int   getLastAgentIndex()=0;

    virtual int setParams(const std::string sParams)=0;
    virtual int flushDeadSpace()=0;

    virtual int  updateEvent(int EventID, char *pData, float fT)=0;
    virtual void flushEvents(float fT)=0;

    virtual uint     getAgentLifeState(int iAgentIndex)=0;
    virtual int      getAgentCellIndex(int iAgentIndex)=0;
    virtual idtype   getAgentID(int iAgentIndex)=0;
    virtual gridtype getAgentCellID(int iAgentIndex)=0;
    virtual float    getAgentBirthTime(int iAgentIndex)=0;
    virtual uchar    getAgentGender(int iAgentIndex)=0;

    virtual hid_t    getAgentQDFDataType()=0;

    virtual bool hasAction(const std::string sAction) = 0;    
    virtual bool hasParam(const std::string sParam) = 0;    

    virtual WELL512 **getWELL() = 0;
    
    //@@ to be removed
    /*
    virtual void setQDFVersionIn(int iV)=0;
    virtual int  getQDFVersionIn()=0;
    virtual void setQDFVersionOut(int iV)=0;
    virtual int  getQDFVersionOut()=0;
    */ 
    //@@ until here.

    virtual int addPopSpecificAgentData(int iAgentIndex, char **ppData)=0;

};

#endif
