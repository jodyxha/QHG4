// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ****************************************************************************
//  File: GlyphGeometry.C
// ****************************************************************************

#include <sstream>
#include <ostream>
#include <vtkPoints.h>
#include <vtkUnsignedCharArray.h>
#include <vtkPolyData.h>
#include <vtkDataSetMapper.h>
#include <vtkTransform.h>
#include <vtkTransformFilter.h>

#include <DebugStream.h>

#include "GlyphGeometry.h"


// ****************************************************************************
//  Method: GlyphGeometry constructor
//
//  Arguments:
//      sTypeName    name of glyph type ("Pie", "Bars", "Boxes")
//      colvec       array with colors for the values
//    
//  Programmer: jodyxha
//  Creation:   Wed Oct 6 11:02:10 PDT 2021
//
// ****************************************************************************

GlyphGeometry::GlyphGeometry(std::string sTypeName, colvec &vColors) 
    : m_newPoints(NULL),
      m_newCols(NULL),
      m_newPolyData(NULL),
      m_iNumValues(0),
      m_pValues(NULL),
      m_sTypeName(sTypeName),
      m_vColors(vColors)  
{
   
    m_pPos  = new double[3];
    m_pNorm = new double[3];
    m_pScale = new double[3];
    
}
 
   
// ****************************************************************************
//  Method: GlyphGeometry destructor
//
//  Programmer: jodyxha
//  Creation:   Wed Oct 6 11:02:10 PDT 2021
//
// ****************************************************************************

GlyphGeometry::~GlyphGeometry() 
{
    cleanVTKObjects();

    delete[] m_pPos;
    delete[] m_pNorm;
    delete[] m_pScale;
    delete[] m_pValues;
}


// ****************************************************************************
//  Method:  GlyphGeometry::cleanVTKObjects
//
//  Purpose: 
//    clean up some of the VTK objects
//
//  Programmer: jodyxha
//  Creation:    Wed Oct 6 11:02:10 PDT 2021
//
// ****************************************************************************
void 
GlyphGeometry::cleanVTKObjects() 
{
    if (m_newPoints != NULL) {
        m_newPoints->Delete();
    } 
    if (m_newCols != NULL) {
        m_newCols->Delete();
    } 
    if (m_newPolyData != NULL) {
        m_newPolyData->Delete();
    } 
}


// ****************************************************************************
//  Method:  GlyphGeometry::createPolyData
//
//  Purpose:
//    create the vtk objects needed to drw a glyph,
//    then call createGlyph() to fill them to draw 
//
//  Arguments:
//      pPoint      coordinates of glyph
//      pNormal     desired notmal o glyph
//      fScale      scaliung for glyph
//      iNumValues  number of values to displayed by glyph
//      pValues     the vlues to be displayed by glyph
//
//  Returns:        a vtkPolyData for the glyph
//
//  Programmer: jodyxha
//  Creation:   Wed Oct 6 11:02:10 PDT 2021
//
// ****************************************************************************
vtkPolyData *
GlyphGeometry::createPolyData(const double *pPoint, 
                                           const double *pNormal, 
                                           double fScale, 
                                           int iNumValues, 
                                           const double *pValues) 
{

    int iResult = 0;

    // we need to do this at least for newCols
    // i think  
    //      m_newPolyData->GetPointData()->SetScalars(m_newCols);
    // keeps a smart pointer to m_newCols. If we change this later it affects the previously createdactors.
    // but if we delete it, the previous actors are not affected
 
    m_iNumValues = iNumValues;
    m_pValues = new double[m_iNumValues];
    cleanVTKObjects();

    m_newPoints = vtkPoints::New();

    m_newCols   = vtkUnsignedCharArray::New();
    m_newCols->SetNumberOfComponents(3);
    m_newCols->SetName("Colors");

    m_newPolyData  = vtkPolyData::New();


    // createGlyph will fill m_newPolyData with points and cells
    //    debug1 << "QHG: [GlyphGeometry::createPolyData] creating glyph (" << m_sTypeName << ")" << endl << std::flush;

    iResult = createGlyph(iNumValues, pValues);


    // save the data (we currently don't use it, but who knows?)
    memcpy(m_pPos,  pPoint,  3*sizeof(double));
    memcpy(m_pNorm, pNormal, 3*sizeof(double));
    m_pScale[0] = fScale;
    m_pScale[1] = fScale;
    m_pScale[2] = fScale;
    memcpy(m_pValues, pValues,  iNumValues*sizeof(double));

    return m_newPolyData;
}



