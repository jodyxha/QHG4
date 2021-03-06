#include "Geo2LookUp.h"
#include <cmath>
#include <cstdio>
//-----------------------------------------------------------------------------
// constructor
//
Geo2LookUp::Geo2LookUp(double dMinLevel, double dSeaLevel, double dMaxLevel) 
    :   LookUp(dMinLevel, dMaxLevel),
        m_dSeaLevel(dSeaLevel) {

}

//-----------------------------------------------------------------------------
// getColor
//
void Geo2LookUp::getColor(double dValue, double &dRed, double &dGreen, double &dBlue, double &dAlpha) {

    if (std::isnan(dValue)) {
        dRed = 1.0;
        dGreen = 1.0;
        dBlue  = 1.0;
        dAlpha = 0.0;

    } else if (dValue < m_dMinLevel) {
        dRed   = 0.0;
        dGreen = 0.0;
        dBlue  = 1.0;
        dAlpha = 1.0;
 
   } else if (dValue < m_dSeaLevel) { 
        dValue -= m_dMinLevel;
        dValue /= (m_dSeaLevel-m_dMinLevel);
        dRed   = 0.0;
        dGreen = dValue;
        dBlue  = 1.0;
        dAlpha = 1.0;
        
    } else if (dValue > m_dMaxLevel) {
        dRed   = 1.0;
        dGreen = 0.99;
        dBlue  = 0.99;
        dAlpha = 1.0;
        
    } else {
        
        dValue -= m_dSeaLevel;
        dValue /= m_dMaxLevel - m_dSeaLevel;

        if (3*dValue < 1) {
            dRed   = 3*dValue;
            dGreen = 0.6;
            dBlue  = 0.0;

        } else if (3*dValue < 2) {
            dRed   = 1.0; 
            dGreen = 1.2-1.8*dValue;
            dBlue  = 0.0;
 
       } else {
            dRed   = 1.0;
            dGreen = 2.97*dValue - 1.98;
            dBlue  = 2.97*dValue - 1.98;
        }
        
        dAlpha = 1.0;
    }
}
