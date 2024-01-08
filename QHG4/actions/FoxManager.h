#ifndef __FOXMANAGER_H__
#define __FOXMANAGER_H__

#include "Action.h"
#include "ParamProvider2.h"
#include "ArrayShare.h"

const static std::string ATTR_FOXMAN_NAME   = "FoxManager";


template<typename T>
class FoxManager {
public:
    FoxManager(SPopulation<T> *pPop, SCellGrid *pCG, std::string sID,
               const std::string sNameFoxCount,    const std::string sNameRabbitMassAvail, 
               const std::string sNameRabbitCells, const std::string sNameDeadRabbits);

    virtual ~FoxManager();

    virtual int preLoop();
    virtual int initialize(float fT);
    virtual int finalize(float fT);

protected:
    ArrayShare *m_pAS;
    int     m_iNumCells;
    // owned shared arrays
    int    *m_aiFoxCount;
    std::vector<std::pair<int,int>> *m_avDeadRabbits;

    // names for foreign shared arrays
    const std::string m_sNameFoxCount;
    const std::string m_sNameRabbitMassAvail;
    const std::string m_sNameRabbitCells;
    const std::string m_sNameDeadRabbits;
 
    // foreign shared arrays
    double *m_adRabbitMassAvail;
    std::vector<std::pair<int, double>> *m_vLocRabbitIDs;

    // fox IDs per cell
    std::vector<int> *m_avLocFoxIDs;
 
    omp_lock_t* m_aFLocks;
};

#endif
