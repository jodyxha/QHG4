#include "RainbowLookUp.h"
#include <cmath>
#include <cstdio>
//-----------------------------------------------------------------------------
// constructor
//
RainbowLookUp::RainbowLookUp(double dMinLevel, double dMaxLevel) 
    :   LookUp(dMinLevel, dMaxLevel) {

}

//-----------------------------------------------------------------------------
// getColor
//
void RainbowLookUp::getColor(double dValue, double &dRed, double &dGreen, double &dBlue, double &dAlpha) {

    if (std::isnan(dValue)) {
        dRed   = 1.0;
        dGreen = 1.0;
        dBlue  = 1.0;
        dAlpha = 0.0;
    } else if (std::isinf(dValue)) {
        dRed   = 0.0;
        dGreen = 0.0;
        dBlue  = 0.0;
        dAlpha = 0.0;
    } else if (dValue <= m_dMinLevel+((m_dMaxLevel-m_dMinLevel)*1e-8)) {
        //        dRed   = 0.59;
        //        dGreen = 0.66;
        //        dBlue  = 0.59;
        dRed   = 0.358;
        dGreen = 0.098;
        dBlue  = 0.663;

        dAlpha = 1.0;
        
    } else if (dValue >= m_dMaxLevel) {
        dRed   = 1.0;
        dGreen = 0.0;
        dBlue  = 1.0;

        dAlpha = 1.0;
    } else {
        
        dValue -= m_dMinLevel;
        dValue /= m_dMaxLevel - m_dMinLevel;

        if (4*dValue < 1) {
            dRed   = 0.0; 
            dGreen = 4*dValue;
            dBlue  = 1.0;

        } else if (4*dValue < 2) {
            dRed   = 0.0; 
            dGreen = 1.0;
            dBlue  = 2 - 4*dValue;
            
        } else if (4*dValue < 3) {
            dRed   = 4*dValue - 2; 
            dGreen = 1.0;
            dBlue  = 0.0;

        } else {
            dRed   = 1.0;
            dGreen = 4-4*dValue;
            dBlue  = 0.0;
        }
        
        dAlpha = 1.0;
    }
}



//-----------------------------------------------------------------------------
// getColor
//
void RainbowLookUp::getColor(double dValue, unsigned char &uRed, unsigned char &uGreen, unsigned char &uBlue, unsigned char &uAlpha) {
    double dRed;
    double dGreen;
    double dBlue;
    double dAlpha;

    getColor(dValue, dRed, dGreen, dBlue, dAlpha);
    uRed   = 255*dRed;
    uGreen = 255*dGreen;
    uBlue  = 255*dBlue;
    uAlpha = 255*dAlpha;
}


