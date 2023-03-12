#ifndef __SHEEPPOP_H__
#define __SHEEPPOP_H__

#include <hdf5.h>

#include "SPopulation.h"

#include "WeightedMove.h"
#include "SingleEvaluator.h"
#include "MultiEvaluator.h"
#include "SheepManager.h"
#include "Starver.h"
#include "AnimalReproducer.h"

// here we just happen to know the share names
// perhaps we should store them in a singleton?

// shares created by GrassPop
#define SHARE_GRASS_MASS_AVAILABLE  "grass_mass_available"
#define SHARE_GRASS_MASS_CONSUMED   "grass_mass_consumed"

struct SheepAgent : Agent {
    double m_dMass;
    double m_dBabyMass;
};

class SheepPop : public SPopulation<SheepAgent> {

 public:
    SheepPop(SCellGrid *pCG, PopFinder *pPopFinder, int iLayerSize, IDGen **apIDG, uint32_t *aulState, uint *aiSeeds);
    ~SheepPop();

    int preLoop();
    int addPopSpecificAgentData(int iAgentIndex, char **ppData);
    void addPopSpecificAgentDataTypeQDF(hid_t *hAgentDataType);
    int makePopSpecificOffspring(int iAgent, int iMother, int iFather);

 protected:

    SingleEvaluator<SheepAgent>   *m_pSEGrass;
    WeightedMove<SheepAgent>      *m_pWM;
    SheepManager<SheepAgent>      *m_pSM;
    Starver<SheepAgent>           *m_pRS;
    AnimalReproducer<SheepAgent>  *m_pAR;
    

  
    double    *m_pdGrassMassAvailable; 
    double    *m_pdGrassMassConsumed;
    
  
    double    *m_adPreferences;

};

#endif
