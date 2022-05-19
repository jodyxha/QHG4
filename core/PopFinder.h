#ifndef __POPFINDER_H__
#define __POPFINDER_H__

#include <string>
#include "PopBase.h"


class PopFinder {
public:
    //    virtual PopBase *getPopByID(idtype iSpeciesID)=0;
    virtual PopBase *getPopByName(const std::string sSpeciesName)=0;
};

#endif
