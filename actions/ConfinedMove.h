#ifndef __CONFINEDMOVE_H__
#define __CONFINEDMOVE_H__

#include "Action.h"
#include "ParamProvider2.h"

#define ATTR_CONFINEDMOVE_NAME "ConfinedMove"
#define ATTR_CONFINEDMOVE_X_NAME "ConfinedMove_x"
#define ATTR_CONFINEDMOVE_Y_NAME "ConfinedMove_y"
#define ATTR_CONFINEDMOVE_R_NAME "ConfinedMove_r"


template<typename T>
class ConfinedMove : public Action<T> {
    
 public:
    ConfinedMove(SPopulation<T> *pPop, SCellGrid *pCG, std::string sID, std::vector<int>** vMoveList);
    virtual ~ConfinedMove();

    int preLoop();

    int finalize(float fTime);

    int extractParamsQDF(hid_t hSpeciesGroup);
    int writeParamsQDF(hid_t hSpeciesGroup);

    virtual int tryGetParams(const ModuleComplex *pMC);  
   int modifyParams(const std::string sAttrName, double dValue);

    bool isEqual(Action<T> *pAction, bool bStrict);
    


 protected:
    double m_dX;
    double m_dY;
    double m_dR;
    std::vector<int>** m_vMoveList;
    bool* m_bAllowed;
    bool m_bAbsorbing;

    static const char *asNames[];
};

#endif
