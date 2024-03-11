#ifndef __GRASSPOP_H__
#define __GRASSPOP_H__

#include <hdf5.h>

#include "SPopulation.h"
#include "GrassManager.h"

const static std::string  SHARE_GRASS_MASS_AVAILABLE = "grass_mass_available";
const static std::string  SHARE_GRASS_MASS_CONSUMED  = "grass_mass_consumed";

struct GrassAgent : Agent {
    double m_dMass;
};

class GrassPop : public SPopulation<GrassAgent> {

 public:
    GrassPop(SCellGrid *pCG, PopFinder *pPopFinder, int iLayerSize, IDGen **apIDG, uint32_t *aulState, uint *aiSeeds);
    ~GrassPop();

    int addPopSpecificAgentData(int iAgentIndex, char **ppData);
    void addPopSpecificAgentDataTypeQDF(hid_t *hAgentDataType);

    int writeAdditionalDataQDF(hid_t hSpeciesGroup);
 protected:

    GrassManager<GrassAgent> *m_pGM;


    double *m_pdGrassAvailable;
    double *m_pdGrassConsumed;


};


#endif
