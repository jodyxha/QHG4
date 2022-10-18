#ifndef __FADETO_H__
#define __FADETO_H__

#include "LookUp.h"

class FadeToLookUp : public LookUp {

public:
    FadeToLookUp(double dMin, double dMax,
                 double dR0, double dG0, double dB0, double dA0,
                 double dR1, double dG1, double dB1, double dA1);
    
    virtual void getColor(double dValue, double &dRed, double &dGreen, double &dBlue, double &dAlpha);
    virtual void getColor(double dValue, unsigned char &uRed, unsigned char &uGreen, unsigned char &uBlue, unsigned char &uAlpha);

protected:
   double m_dR0;
   double m_dG0;
   double m_dB0;
   double m_dA0;
   double m_dR1;
   double m_dG1;
   double m_dB1;
   double m_dA1;
};



#endif
