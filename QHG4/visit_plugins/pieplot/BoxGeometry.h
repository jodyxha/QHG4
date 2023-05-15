// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ****************************************************************************
//  File: BoxGeometry.h
// ****************************************************************************

#ifndef __BOXGEOMETRY_H__
#define __BOXGEOMETRY_H__

#include "vtkPoints.h"
#include "vtkUnsignedCharArray.h"
#include "GlyphGeometry.h"

// ********************************************************
// Class: BoxGeometry
//
// Purpose:
//   PieGeometry implements GlyphGeometry and creates
//   the vtkPoints and triangles for a box diagram.
//
// Programmer: jodyxha
// Creation: August 29, 2021
//
// Modifications:
//
// ******************************

class BoxGeometry : public GlyphGeometry 
{
public:
    BoxGeometry(double fScaleX, double fScaleY, double fBorder, colvec &vColors);

    virtual ~BoxGeometry();

protected: 
    virtual  int createGlyph(int iNumValues, const double *pValues);
    int  createValueBoxes(int iNumValues, const double *pValues);
    void createVertBorders(double fXOff);
    void createHoriBorders(double fYOff);
    void drawBox(double iXMin, double iYMin,double iXMax, double iYMax, const double *aCurCol);

    double m_fScaleX;
    double m_fScaleY;
    double m_fBorder; 
};

#endif
