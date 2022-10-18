#ifndef __BINFUNC_H__
#define __BINFUNC_H__

class BinFunc {
public:
    BinFunc(int iNumBins): m_iNumBins(iNumBins) { m_piBins = new uint[m_iNumBins];};
    virtual ~BinFunc() { delete[] m_piBins;};

    virtual int   calcBin(float v) = 0;
    virtual uint  getNumBins() {return m_iNumBins;};
    virtual uint *getBins() {return m_piBins;};
    
    virtual stringvec &getHeaders(stringvec &vHeaders) = 0;
    virtual stringvec &getValues(stringvec &vValues) = 0;
protected:
    uint     *m_piBins;
    uint      m_iNumBins;

};



#endif
