#ifndef __DIRMOVE_H__
#define __DIRMOVE_H__

#include "Action.h"
#include "ParamProvider2.h"
#include "PolyLine.h"
#include "Geography.h"

#define ATTR_DIRMOVE_NAME "DirMove"

template<typename T>
class DirMove : public Action<T> {
    
 public:
    DirMove(SPopulation<T> *pPop, SCellGrid *pCG, std::string sID);
    ~DirMove();
    int execute(int iA, float fT);

    virtual int extractAttributesQDF(hid_t hSpeciesGroup);
    int writeAttributesQDF(hid_t hSpeciesGroup);


    bool isEqual(Action<T> *pAction, bool bStrict);
       

 protected:
 
    Geography *m_pGeography;
};

#endif
