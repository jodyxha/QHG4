
#include "WELL512.h"  
#include "WELLUtils.h"  
#include "Permutator.h"  
#include "stdstrutils.h"
#include "stdstrutilsT.h"
#include "RandomGroupSplitter.h"  

RandomGroupSplitter::RandomGroupSplitter(WELL512 *pWELL, uint iMinSize, uint iMaxSize)
    : m_pWELL(pWELL),
      m_iMinSize(iMinSize),
      m_iMaxSize(iMaxSize) {
      }

int RandomGroupSplitter::split(intvec &vOriginal, intvec &vSplitOff) {
    int iResult = -1;
    stdprintf("min: %u, max %u\n", m_iMinSize, m_iMaxSize);
    //    if ((m_iMinSize <= vOriginal.size()) && (m_iMaxSize > vOriginal.size())) {
    if (m_iMaxSize < vOriginal.size()) {
        iResult = 0;
        int iSize = m_pWELL->wrandi(m_iMinSize, m_iMaxSize);
     
        stdprintf("Size is %d\n", iSize);
        
        WELLUtils::showState(m_pWELL);
        Permutator *pP = Permutator::createInstance(iSize);
        const uint *a = pP->permute(vOriginal.size(), iSize, m_pWELL);
        stdprintf("a:[ ");
        for (int i = 0; i < iSize; i++) {
            stdprintf("%d ", a[i]);
        }
        stdprintf("]\n");
        uint *b = new uint[iSize];
        memcpy(b, a, iSize*sizeof(uint));

        std::sort(b, b+iSize);
        for (int i = iSize -1; i >= 0; i--) {
            vSplitOff.push_back(vOriginal[b[i]]);
            //stdprintf("Removing element %d from %bv\n", b[i], vOriginal);
            vOriginal.erase(vOriginal.begin()+b[i]);
        }
        std::sort(vSplitOff.begin(), vSplitOff.end());
        delete pP;
        
    } else {
        stdprintf("Size requirement failed: min %d, max %d vOriginal\n");
    }
    return iResult;
}

