#include "FadeToLookUp.h"
#include <cmath>
#include <cstdio>

//-----------------------------------------------------------------------------
// constructor
//
FadeToLookUp::FadeToLookUp(double dMinLevel, double dMaxLevel,
                           double dR0, double dG0, double dB0, double dA0,
                           double dR1, double dG1, double dB1, double dA1)
 
    :   LookUp(dMinLevel, dMaxLevel),
        m_dR0(dR0),
        m_dG0(dG0),
        m_dB0(dB0),
        m_dA0(dA0),
        m_dR1(dR1),
        m_dG1(dG1),
        m_dB1(dB1),
        m_dA1(dA1) {
}


//-----------------------------------------------------------------------------
// getColor
//
void FadeToLookUp::getColor(double dValue, double &dRed, double &dGreen, double &dBlue, double &dAlpha) {
    if (std::isnan(dValue)) {
        dRed   = 1.0;
        dGreen = 1.0;
        dBlue  = 1.0;
        dAlpha = 0.0;
     } else if (dValue <= m_dMinLevel) {
        dRed   = m_dR0;
        dGreen = m_dG0;
        dBlue  = m_dB0;
        dAlpha = m_dA0;
        
    } else if (dValue >= m_dMaxLevel) {
        dRed   = m_dR1;
        dGreen = m_dG1;
        dBlue  = m_dB1;
        dAlpha = m_dA1;

    } else {
        // normalize dvalue
        dValue -= m_dMinLevel;
        dValue /= m_dMaxLevel - m_dMinLevel;
        
        dRed   = m_dR0*(1-dValue) + m_dR1*dValue;
        dGreen = m_dG0*(1-dValue) + m_dG1*dValue;
        dBlue  = m_dB0*(1-dValue) + m_dB1*dValue;
        dAlpha = m_dA0*(1-dValue) + m_dA1*dValue;
        
    }
}


//-----------------------------------------------------------------------------
// getColor
//
void FadeToLookUp::getColor(double dValue, unsigned char &uRed, unsigned char &uGreen, unsigned char &uBlue, unsigned char &uAlpha) {
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

