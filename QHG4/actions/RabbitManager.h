#ifndef __RABBITMANAGER_H__
#define __RABBITMANAGER_H__

#include "Action.h"
#include "ParamProvider2.h"
#include "ArrayShare.h"

#define ATTR_RABBITMAN_NAME "RabbitManager"

typedef std::pair<int, double>  cellnm;  // count and mass in cell
typedef std::vector<cellnm>     cellnmvec;

typedef std::pair<int,int>      agcell;    //agent ID and cell ID 
typedef std::vector<agcell>     agcellvec; // 

template<typename T>
class RabbitManager : public Action<T> {
public:
    RabbitManager(SPopulation<T> *pPop, SCellGrid *pCG, std::string sID, 
                  const std::string sNameRabbitMassAvail, const std::string sNameGrassMassConsumed, 
                  const std::string sNameRabbitLocIDs,    const std::string sNameFoxCount, 
                  const std::string sNameGrassMassAvail,  const std::string sNameDeadRabbits,
                  double *pdPreferences);
   
    virtual ~RabbitManager();

    virtual int preLoop();
    virtual int initialize(float fT);
    virtual int finalize(float fT);

    virtual bool isEqual(Action<T> *pAction, bool bStrict);

    //@@TODO: fill a preference array from +Grass (done) and -Fox
    //@@TODO: implement Starver, implement RabbitPop

protected:
    ArrayShare *m_pAS;
    int     m_iNumCells;
    double *m_adRabbitMassAvail;
    double *m_adGrassMassConsumed;
    int    *m_aiFoxCount;
    double *m_adGrassMassAvail;
    cellnmvec *m_avRabbitLocIDs;
    agcellvec *m_avRabbitDead;
 
    double *m_pdPreferences;
    const std::string m_sNameRabbitMassAvail;
    const std::string m_sNameGrassMassConsumed;
    const std::string m_sNameRabbitLocIDs;
    const std::string m_sNameFoxCount;
    const std::string m_sNameGrassMassAvail;
    const std::string m_sNameRabbitDead;
 
    
 
    omp_lock_t* m_aRLocks;

    int setAvailableRabbitMass();
    int feedRabbits();
    int makeCellVecsRabbit();

};

#endif
