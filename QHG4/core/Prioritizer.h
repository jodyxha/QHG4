#ifndef __PRIORITIZER_H__
#define __PRIORITIZER_H__

#include <map>
#include <set>
#include <vector>
#include <string>
#include <hdf5.h>

#include "types.h"
#include "ParamProvider2.h"


template <typename A> class Action;

template <typename A>
class Prioritizer {
public:
    typedef std::map<uint, std::vector< Action<A>* > > actlist;
    typedef std::map<std::string, Action<A>* > namelist;

    Prioritizer() {};
    // virtual destructor for nice clean up
    virtual ~Prioritizer() {};

    // associate priority values with methods
    virtual int setPrio(uint iPrio, std::string name);

    // associate names with actions
    virtual int addAction(Action<A> *act);
    //    virtual int addAction(std::string name, Action<A> *act);

    // remove an action completely (can't be undone - must call addAction() again)
    virtual int removeAction(std::string name);

    // return true if action with given name exists
    virtual bool hasAction(std::string name);

    // return true if attribute with given name exists
    virtual bool hasParam(std::string name);

    // return highest priority value
    virtual uint getMaxPrio() { return m_prios.rbegin()->first;};

    // return highest priority value
    virtual uint getPrios( std::set<uint> &vPrios);

    uint getNumActionsForPrio(uint iPrio) { return (uint)m_prios[iPrio].size();};

    Action<A> *getAction(uint iPrio, int iWhich) {return m_prios[iPrio][iWhich];};

    int extractActionParamsQDF(hid_t hSpeciesGroup); // for qdf
    int writeActionParamsQDF(hid_t hSpeciesGroup); // for qdf

    int getActionAttributes(const modulemap &mModules); // for stringmap

    int dumpActionStatesQDF(hid_t hSpeciesGroup); // for qdf
    int restoreActionStatesQDF(hid_t hSpeciesGroup); // for qdf

    int modifyAttributes(const std::string sAttrName, double dValue);
    
    // compare all Action elements 
    bool isEqual(Prioritizer<A> *pP, bool bStrict);

    void showAttributes();

    inline bool empty() {return m_prios.empty();};
protected:
    namelist m_names;
    actlist  m_prios;
};

#endif
