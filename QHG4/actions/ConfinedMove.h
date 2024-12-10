#ifndef __CONFINEDMOVE_H__
#define __CONFINEDMOVE_H__

#include "Action.h"
#include "ParamProvider2.h"

const static std::string ATTR_CONFINEDMOVE_NAME   = "ConfinedMove";
const static std::string ATTR_CONFINEDMOVE_X_NAME   = "ConfinedMove_x";
const static std::string ATTR_CONFINEDMOVE_Y_NAME   = "ConfinedMove_y";
const static std::string ATTR_CONFINEDMOVE_R_NAME   = "ConfinedMove_r";


template<typename T>
class ConfinedMove : public Action<T> {
    
 public:
    ConfinedMove(SPopulation<T> *pPop, SCellGrid *pCG, std::string sID/*, std::vector<int>** vMoveList*/);
    virtual ~ConfinedMove();

    virtual int preLoop();

    virtual int finalize(float fTime);

    virtual int extractAttributesQDF(hid_t hSpeciesGroup);
    virtual int writeAttributesQDF(hid_t hSpeciesGroup);

    virtual int tryGetAttributes(const ModuleComplex *pMC);  
    virtual int modifyAttributes(const std::string sAttrName, double dValue);

    virtual bool isEqual(Action<T> *pAction, bool bStrict);
    


 protected:
    double m_dX;
    double m_dY;
    double m_dR;
    std::vector<int>** m_vMoveList;
    bool* m_bAllowed;
    bool m_bAbsorbing;

    static const std::string asNames[];
};

#endif
