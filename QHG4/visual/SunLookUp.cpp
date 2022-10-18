#include "SunLookUp.h"
#include <cmath>
#include <cstdio>
//-----------------------------------------------------------------------------
// constructor
//
SunLookUp::SunLookUp(double dMinLevel, double dMaxLevel) 
    :   LookUp(dMinLevel, dMaxLevel) {

}

//-----------------------------------------------------------------------------
// getColor
//
void SunLookUp::getColor(double dValue, double &dRed, double &dGreen, double &dBlue, double &dAlpha) {

    if (std::isnan(dValue)) {
        dRed   = 0.0;
        dGreen = 0.1;
        dBlue  = 0.2;
        dAlpha = 0.0;
    } else if (std::isinf(dValue)) {
        dRed   = 0.0;
        dGreen = 0.0;
        dBlue  = 0.0;
        dAlpha = 0.0;
    } else if (dValue <= m_dMinLevel) {
        dRed   = 0.0;
        dGreen = 0.0;
        dBlue  = 0.0;
        dAlpha = 1.0;
        
    } else if (dValue >= m_dMaxLevel) {
        dRed   = 1.0;
        dGreen = 1.0;
        dBlue  = 1.0;
        dAlpha = 1.0;
    } else {
        
        dValue -= m_dMinLevel;
        dValue /= m_dMaxLevel - m_dMinLevel;
      
            dRed   = dValue;
            dGreen = dValue;
            dBlue  = dValue;
      
        
        dAlpha = 1.0;
    }
}
