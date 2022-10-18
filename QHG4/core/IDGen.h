#ifndef __IDGEN_H__
#define __IDGEN_H__
/****************************************************************************
 * IDGen is used for globally unique IDs
 *   iB     is the base, i.e. the lowest ID to be created
 *   iOffs  is an offset to distinguish IDs of different threads;
 *          usually the number of the thread (for multi node apps a global thread number)
 *   iStep  increment of two consecutive IDs of a thread;
 *          typically the total number of threads
 ****************************************************************************/
 
#include <omp.h>
#include "types.h"

class IDGen {
public:
    IDGen(idtype iBase, idtype iOffs, idtype iIncrement) 
        : m_iOffs(iOffs),
          m_iIncrement(iIncrement),
          m_iCur(0) {

        setBase(iBase);
    }

    void setBase(idtype iBase) { m_iCur = iBase + m_iOffs - m_iIncrement; }; // subtract increment because for next ID we add it again
    void setData(idtype iBase, idtype iOffs, idtype iIncrement) { m_iOffs=iOffs; m_iIncrement=iIncrement; setBase(iBase);};

    inline idtype getID() { return m_iCur+= m_iIncrement; };

    // dump&restore
    idtype getCur() { return m_iCur;};
    void   setCur(idtype iCur) { m_iCur = iCur;};
private:
    idtype m_iOffs;
    idtype m_iIncrement;
    idtype m_iCur;
};

#endif
