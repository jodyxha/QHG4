#ifndef __PDHUNTING_H__
#define __PDHUNTING_H__

#include "Action.h"
#include "ParamProvider2.h"
#include "PreyDistributor.h"

const static std::string ATTR_PDHUNTING_NAME             = "PDHunting";
const static std::string ATTR_PDHUNTING_RELATIONS_NAME   = "PDHunting_relations";

#define NAME_LEN 1024

typedef std::pair<double,double>          doublepair;
typedef std::map<std::string, doublepair> relation;

class MassInterface;
class PopFinder;


template<typename T>
class PDHunting : public Action<T> {
public:
    PDHunting(SPopulation<T> *pPop, SCellGrid *pCG, std::string sID, WELL512 **apWELL, PopFinder *pPopFinder);
    virtual ~PDHunting();

    int preLoop();
    int initialize(float fTime);
    int execute(int iA, float fT);
    int postLoop();
    
    virtual int extractAttributesQDF(hid_t hSpeciesGroup);
    int writeAttributesQDF(hid_t hSpeciesGroup);

    virtual int tryGetAttributes(const ModuleComplex *pMC);  

    bool isEqual(Action<T> *pAction, bool bStrict);
    
    void showAttributes();

protected:
    int relationFromString(std::string sRelations);

    WELL512 **m_apWELL;
    double m_dEfficiency;
    double m_dUsability;
    std::map<std::string, MassInterface *> m_mpMIPrey;
    MassInterface *m_pMIPred;
    PopFinder *m_pPopFinder;
    // maybe unneeded
    std::map<std::string, PopBase *> m_mpPreyPop;

    relation   m_mRelations;
    preyratio *m_pPreyEff;
    std::string m_sPredName;

    std::string m_sRelationInput;

};


#endif
