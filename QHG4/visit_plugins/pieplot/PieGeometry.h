// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ****************************************************************************
//  File: PieGeometry.h
// ****************************************************************************

#ifndef __PIEGEOMETRY_H__
#define __PIEGEOMETRY_H__

#include "vtkPoints.h"
#include "vtkUnsignedCharArray.h"
#include "GlyphGeometry.h"

// ********************************************************
// Class: PieGeometry
//
// Purpose:
//   PieGeometry implements GlyphGeometry and creates
//   the vtkPoints and triangles for a pie diagram.
//
// Programmer: jodyxha
// Creation: August 29, 2021
//
// Modifications:
//
// ********************************************************

class PieGeometry : public GlyphGeometry 
{
public:
    PieGeometry(int iNumSectors, double dR1, double dR2, colvec &vColors);
 
    virtual ~PieGeometry();
    
protected: 
    virtual  int createGlyph(int iNumValues, const double *pValues);
    
    int createBlankPie();
 
    int createRegularPieNew(int iNumValues, const double *pValues);
    
    int *makePartition(int iNumValues, const double *pValues, double fSum);

    void  createPieSegment(double alpha_prev, double alpha_cur, const double *curCol);

    uint m_iNumSectors;
    double m_dR1;
    double m_dR2;
};
#endif
