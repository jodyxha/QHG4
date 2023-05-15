// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ****************************************************************************
//  File: GlyphManager.h
// ****************************************************************************

#ifndef __GLYPHMANAGER_H__
#define __GLYPHMANAGER_H__

#include <vector>

#include "GlyphGeometry.h"

// ********************************************************
// Class: GlyphManager
//
// Purpose:
//   GlyphManager holds the plot specific attributes and
//   can create a vtkPolyData for a glyph
//
// Programmer: jodyxha
// Creation: August 29, 2021
//
// Modifications:
//
// ******************************

class GlyphManager 
{

public:
    static GlyphManager *createInstance();
    static vtkTransform *calcTransform(const double *pPoint, const double *pNormal, const double *pScale, bool bUpright);
    
    vtkPolyData *createGlyph(uint iType, const double *pPoint, const double *pNormal, double fScale, int iNumValues, const double *pValues);

    virtual ~GlyphManager();


    void setPieAtts(int iNumSectors, float fRadPie, float fPieBorder);
    void setBarAtts(float fScaleX, float fScaleY, float fBarBorder);
    void setBoxAtts(float fScaleX, float fScaleY, float fBoxBorder);

    void setGlyphColors(colvec &vColors);


protected:
    GlyphManager();
    int init();
    void cleanUp();


    int   m_iNumSectors;
    float m_fRadiusPie;
    float m_fRadiusBorder;
    float m_fBarScaleY;
    float m_fBarScaleX;
    float m_fBarBorder;
    float m_fBoxScaleX;
    float m_fBoxScaleY;
    float m_fBoxBorder;

    colvec m_vGlyphColors;

private:
    static GlyphManager *_instance;
};


#endif
