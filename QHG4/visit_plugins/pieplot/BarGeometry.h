// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ****************************************************************************
//  File: BarGeometry.h
// ****************************************************************************

#ifndef __BARGEOMETRY_H__
#define __BARGEOMETRY_H__

#include "vtkPoints.h"
#include "vtkUnsignedCharArray.h"
#include "GlyphGeometry.h"

// ********************************************************
// Class: BarGeometry
//
// Purpose:
//   BarGeometry implements GlyphGeometry and creates
//   the vtkPoints and triangles for a bar diagram.
//
// Programmer: jodyxha
// Creation: August 29, 2021
//
// Modifications:
//
// ********************************************************

class BarGeometry : public GlyphGeometry 
{
public:
    BarGeometry(double fScaleX, double fScaleY, double fBorder, colvec &vColors);

    virtual ~BarGeometry();

protected: 
    virtual  int createGlyph(int iNumValues, const double *pValues);
    int  createBars(int iNumValues, const double *pValues);
    void createSideBorders(double fXOff);
    void createBottomBorders();

    double m_fScaleX;
    double m_fScaleY;
    double m_fBorder; 
};

#endif
