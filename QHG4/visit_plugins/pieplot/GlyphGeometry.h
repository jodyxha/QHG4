// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ****************************************************************************
//  File: GlyphGeometry.h
// ****************************************************************************

#ifndef __GLYPHGEOMETRY_H__
#define __GLYPHGEOMETRY_H__

#include <map>
#include "vtkTransform.h"
#include "vtkPolyData.h"


typedef std::vector<const double *> colvec;

// ********************************************************
// Class: GlyphGeometry
//
// Purpose:
//   GlyphGeometry is an abstract base class for the 
//   glyphs used in this plot
//
// Notes:     derived classes must implement the method
//            int   createGlyph(int, const double)
//
// Programmer: jodyxha
// Creation: August 29, 2021
//
// Modifications:
//
// ********************************************************


class GlyphGeometry
{
public:
    GlyphGeometry(std::string sTypeName, colvec &vColors);
    virtual vtkPolyData *createPolyData(const double *pPoint, const double *pNormal, double fScale, int iNumValues, const double *pValues);
    virtual ~GlyphGeometry();
    
    vtkPolyData   *getDataSet() { return m_newPolyData;};
    double *getPos() { return m_pPos;};    
    double *getNorm() { return m_pNorm;};    
    double *getValues() { return m_pValues;};    
    int getNumValues() { return m_iNumValues;};
    double *getScale() { return m_pScale;};    
    std::string getTypeName() { return m_sTypeName;};
protected: 
    virtual  int createGlyph(int iNumValues, const double *pValues) = 0;
    void cleanVTKObjects();

     
    vtkPoints            *m_newPoints;
    vtkUnsignedCharArray *m_newCols;
    vtkPolyData          *m_newPolyData;

    uint m_iNumValues;
    double *m_pValues;
    double *m_pPos;
    double *m_pNorm;
    double *m_pScale;

    std::string m_sTypeName;
    colvec m_vColors;
};


#endif
