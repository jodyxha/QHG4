#ifndef __SHEEPMANAGER_H__
#define __SHEEPMANAGER_H__

#include "Action.h"
#include "ParamProvider2.h"
#include "ArrayShare.h"

#define ATTR_SHEEPMAN_NAME "SheepManager"

typedef std::pair<int, double>  cellnm;  // count and mass in cell
typedef std::vector<cellnm>     cellnmvec;

typedef std::pair<int,int>      agcell;    //agent ID and cell ID 
typedef std::vector<agcell>     agcellvec; // 

template<typename T>
class SheepManager : public Action<T> {
public:
    SheepManager(SPopulation<T> *pPop, SCellGrid *pCG, std::string sID, 
                  const std::string sNameGrassMassAvail, const std::string sNameGrassMassConsumed, 
                 //const std::string sNameSheepLocIDs,    const std::string sNameWolfCount, 
                 double *pdPreferences);
   
    virtual ~SheepManager();

    virtual int preLoop();
    virtual int initialize(float fT);
    virtual int finalize(float fT);

    virtual bool isEqual(Action<T> *pAction, bool bStrict);

protected:
    ArrayShare *m_pAS;
    int     m_iNumCells;
    double *m_adGrassMassAvail;
    double *m_adGrassMassConsumed;
    cellnmvec *m_avSheepLocIDs;
    
    double *m_pdPreferences;
    const std::string m_sNameGrassMassAvail;
    const std::string m_sNameGrassMassConsumed;
   
    omp_lock_t* m_aRLocks;

    int feedSheep();
    int makeCellVecsSheep();

};

#endif
