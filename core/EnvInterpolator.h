#ifndef __ENVINTERPOLATOR_H__
#define __ENVINTERPOLATOR_H__

#include <map>
#include <vector>
#include <string>
#include <hdf5.h>

#include "SCellGrid.h"

typedef std::vector<std::string>            name_list;
typedef std::pair<uint, double*>            length_array;
typedef std::map<std::string, length_array> named_arrays;
typedef std::vector<int>                    event_list;

class EnvInterpolator {
public:
    EnvInterpolator(SCellGrid *pCG);
    virtual ~EnvInterpolator();
    
    int readInterpolationData(const std::string sQDFInter);
    bool hasInterpolations();
    int interpolate(int iSteps);

    void showNames();
    void showArrays();
    void setActive(bool bState);
    const event_list &getEvents() {return m_vEvents;};
protected:
    int extractTargets(hid_t hInterp);
    int readArrays(hid_t hInterp);
    int findTargetArrays();
    void cleanUp();

    SCellGrid    *m_pCG;

    named_arrays  m_mArrays;
    name_list     m_vNames;
    named_arrays  m_mTargets;
    event_list    m_vEvents;
    bool          m_bActive;
};
#endif
