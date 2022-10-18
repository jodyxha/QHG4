/*============================================================================
| WELL512
| 
|  Implementation of the WELL512 random number generation from a paper
|  by Chris Lomont: 
|    http://www.lomont.org/math/papers/2008/Lomont_PRNG_2008.pdf
|  
|  Author: Jody Weissmann
\===========================================================================*/ 

#include <cstdlib>
#include <cstring> //for state2String

#include <cmath>  // for normal distr: log, sqrt; and std::isnan
#include <ctime>

#include "WELL512.h"

//-------------------------------------------------------------------------
// constructor 
//   specify state
//
WELL512::WELL512(unsigned int *aulState, uint32_t uiIndex) 
    : m_uiIndex(uiIndex),
      m_iCount(0),
      m_bHavePrevNormal(false),
      m_dPrevNormal(0),
      m_dSigma(0) {

    seed(aulState, uiIndex);
}


//-------------------------------------------------------------------------
// constructor 
//   state "calculated" from current time
//
WELL512::WELL512() 
    : m_uiIndex(0),
      m_iCount(0)  {

    // use number of seconds as seed
    time_t t = time(NULL);
    srand((uint)t);

    uint32_t *p = (uint32_t *)m_aulState;
    for (unsigned int i = 0; i < STATE_SIZE; i++) {
        *p++ = rand();
    }
}


//-------------------------------------------------------------------------
// seed 
//   set state to aulState
//
void WELL512::seed(uint32_t *aulState, uint32_t uiIndex) {
    for (unsigned int i = 0; i < STATE_SIZE; i++) {
        m_aulState[i] = aulState[i];
    }
    m_uiIndex = uiIndex;
}


//-------------------------------------------------------------------------
// wrand
//   next  random number
//   (from a paper by Chris Lomont)
//
uint32_t WELL512::wrand() {
    
    uint32_t a = m_aulState[m_uiIndex];
    uint32_t c = m_aulState[(m_uiIndex+13) & 15];
    uint32_t b = a ^ c ^ (a<<16) ^ (c<<15);
    c = m_aulState[(m_uiIndex+9) & 15];
    c ^= (c>>11);
    a = b ^c;
    m_aulState[m_uiIndex] = a;
    uint32_t d =  a ^ ((a<<5) & (uint32_t)0xDA442D24UL);
    m_uiIndex = (m_uiIndex+15) & 15;
    a = m_aulState[m_uiIndex];
    m_aulState[m_uiIndex] = a ^ b ^ d ^ (a<<2) ^ (b<<18) ^ (c << 28);

    m_iCount++;
    return m_aulState[m_uiIndex];
}
    
//-------------------------------------------------------------------------
// wgauss
//   gaussian distributed random numbers with avg=0.0 and given sigma
//
double WELL512::wgauss(double dSigma) {
    double dResult = 0;
    
    double dS2 = dSigma;
    if ((m_bHavePrevNormal) && (dS2 == m_dSigma)) {
        m_bHavePrevNormal = false;
        dResult =  m_dPrevNormal;
    } else {
        m_dSigma = dS2;
        double v1 = 0;
        double v2 = 0;
        double r2 = 0;
        do {
            v1 = wrandr(-1,1);
            v2 = wrandr(-1,1);
            r2 = v1*v1 + v2*v2;
        } while ((r2 >= 1.0) || (r2 == 0.0));
        double f = sqrt(-2.0*log(r2)/r2)*m_dSigma;
        m_dPrevNormal = v1*f;
        m_bHavePrevNormal = true;
        dResult = v2*f;
    }
    return dResult;
}



//-------------------------------------------------------------------------
// setGaussStates
//   sets the previous normal from wgauss (used by restore)
//
void WELL512::setGaussStates(double dPrevNormal,double dSigma) {

    if (std::isnan(dPrevNormal)) {
        m_bHavePrevNormal = false;
    } else {
        m_bHavePrevNormal = true;
        m_dPrevNormal = dPrevNormal;
    }

    m_dSigma = dSigma;
}


//-------------------------------------------------------------------------
// showState
//
void WELL512::showState() {
    printf("WELL (count %010lu) [%u] ", m_iCount, m_uiIndex);
    for (unsigned int i = 0; i < STATE_SIZE; i++) {
        printf("%08x ", m_aulState[i]);
    }
    printf("\n");
}


//-------------------------------------------------------------------------
// state2String
//
void WELL512::state2String(char *pString) {
    sprintf(pString, "WELL (count %010lu) [%u] ", m_iCount, m_uiIndex);
    for (unsigned int i = 0; i < STATE_SIZE; i++) {
        char sN[64];
        sprintf(sN, "%08x ", m_aulState[i]);
        strcat(pString, sN);
    }
}

