#ifndef __RABBITPOP_H__
#define __RABBITPOP_H__

#include <hdf5.h>

#include "SPopulation.h"

#include "WeightedMove.h"
#include "SingleEvaluator.h"
#include "MultiEvaluator.h"
#include "RabbitManager.h"
#include "Starver.h"
#include "AnimalReproducer.h"
//#include "MoveStats.h"

// here we just happen to know the share names
// perhaps we should store them in a singleton?

// shares created by GrassPop
//const static std::string SHARE_GRASS_MASS_AVAILABLE  = "grass_mass_available";
//const static std::string SHARE_GRASS_MASS_CONSUMED   = "grass_mass_consumed";
// shared arrays created by RabbitPop
const static std::string SHARE_RABBIT_MASS_AVAILABLE = "rabbit_mass_available";
const static std::string SHARE_RABBIT_LOC_IDS        = "rabbit_loc_ids";
// shared array created by FoxPop (@@TODO)
const static std::string SHARE_FOX_COUNT             = "fox_count";
const static std::string SHARE_RABBIT_DEAD           = "rabbit_dead";

struct RabbitAgent : Agent {
    double m_dMass;
    double m_dBabyMass;
};

class RabbitPop : public SPopulation<RabbitAgent> {

 public:
    RabbitPop(SCellGrid *pCG, PopFinder *pPopFinder, int iLayerSize, IDGen **apIDG, uint32_t *aulState, uint *aiSeeds);
    ~RabbitPop();

    int preLoop();
    int addPopSpecificAgentData(int iAgentIndex, char **ppData);
    void addPopSpecificAgentDataTypeQDF(hid_t *hAgentDataType);
    int makePopSpecificOffspring(int iAgent, int iMother, int iFather);

 protected:

    WeightedMove<RabbitAgent>      *m_pWM;
    MultiEvaluator<RabbitAgent>    *m_pME;
    RabbitManager<RabbitAgent>     *m_pRM;
    Starver<RabbitAgent>           *m_pRS;
    AnimalReproducer<RabbitAgent>  *m_pAR;
    
    SingleEvaluator<RabbitAgent>   *m_pSEGrass;

    //    MoveStats<RabbitAgent> *m_pMoveStats;

    double    *m_pdGrassMassAvailable; 
    double    *m_pdGrassMassConsumed;
    cellnmvec *m_avRabbitLocIDs;
    double    *m_adFoxCount;  // actually only a count, but it easier that way to use the MutiEvaluator
    double    *m_adRabbitMassAvail;
    agcellvec *m_avRabbitDead;

    double    *m_adPreferences;

};


#endif
