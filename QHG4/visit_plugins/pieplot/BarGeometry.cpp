// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ****************************************************************************
//  File: BarGeometry.C
// ****************************************************************************

#include <vtkMath.h>
#include <vtkCellArray.h>
#include <vtkFloatArray.h>
#include <vtkObjectFactory.h>
#include <vtkPointData.h>
#include <vtkCellData.h>
#include <vtkPoints.h>
#include <vtkPolyData.h>
#include <vtkTriangle.h>
#include <vtkPolyDataMapper.h>
#include <vtkUnsignedCharArray.h>
#include <vtkTransform.h>
#include <vtkTransformFilter.h>
#include <vtkActor.h>

#include "BarGeometry.h"
#include "GlyphGeometry.h"
#include "GlyphColors.h"

#define SCALE_X   1.0
#define SCALE_Y   1.0
#define BORDER    0.1


// ****************************************************************************
//  Method: BarGeometry constructor
//
//  Arguments:
//      fScaleX     width of bar diagram
//      fScaleY     height of bar diagram
//      fBorder     border width
//      colvec      array with colors for the values
//
//  Programmer: jodyxha
//  Creation:   Wed Oct 6 11:02:10 PDT 2021
//
// ****************************************************************************
BarGeometry::BarGeometry(double fScaleX, double fScaleY, double fBorder, colvec &vColors)
    : GlyphGeometry("Bar", vColors),
      m_fScaleX(fScaleX),
      m_fScaleY(fScaleY),
      m_fBorder(fBorder) 
{

}


// ****************************************************************************
//  Method: BarGometry destructor
//
//  Programmer: jodyxha
//  Creation:   Wed Oct 6 11:02:10 PDT 2021
//
// ****************************************************************************

BarGeometry::~BarGeometry() 
{
}

 
// ****************************************************************************
//  Method:  BarGeometry::createGlyph
//
//  Purpose: 
//    calculate the points and triangles for the pie glyphs
//
//  Arguments:
//      iNumValues   number of values
//      pValues      array of values
//
//  Returns:     result code (0: success; negative number: error)
//
//  Programmer:  jodyxha
//  Creation:    Wed Oct 6 11:02:10 PDT 2021
//
// ****************************************************************************

int 
BarGeometry::createGlyph(int iNumValues, const double *pValues) 
{
    int iResult = 0;
   
    uint iNumTriangles = 6+ 4 * iNumValues;
    uint iNumVerts     = 3*iNumTriangles;

    m_newCols->SetNumberOfComponents(3);
    m_newCols->SetName("Colors");
    
    m_newPoints->SetDataType(VTK_DOUBLE);
    m_newPoints->Allocate(iNumVerts);

    vtkNew<vtkCellArray> newPolys;
    newPolys->Allocate(iNumTriangles);

    iResult = createBars(iNumValues, pValues);
  
    createSideBorders(0);
   
    createSideBorders(m_fScaleX + m_fBorder);
    
    createBottomBorders();

    // shift points
    for (uint i = 0; i < iNumVerts; i++) {
        double *fx = m_newPoints->GetPoint(i);
        fx[0] -= m_fScaleX/2;
        fx[1] -= m_fScaleY/2;
        m_newPoints->SetPoint(i, fx);
    }

    // make triangles
    for (uint i = 0; i < iNumVerts; i+=3) {
        vtkTriangle *triangle1 =vtkTriangle::New();
        triangle1->GetPointIds()->SetId(0, i);
        triangle1->GetPointIds()->SetId(1, i+1);
        triangle1->GetPointIds()->SetId(2, i+2);
        newPolys->InsertNextCell(triangle1);
    }
    
    m_newPolyData = vtkPolyData::New();
    
    m_newPolyData->SetPoints(m_newPoints);
    m_newPolyData->GetPointData()->SetScalars(m_newCols);
    
    //output->GetPointData()->SetNormals(newNormals);
    m_newPolyData->SetPolys(newPolys);  

    return iResult;
}


// ****************************************************************************
//  Method:  BarGeometry::createBars
//
//  Purpose: 
//    (auxiliary method)
//    calculates the points and triangles for the colored bars
//
//  Arguments:
//      iNumValues   number of values
//      pValues      array of values
//
//  Returns:     result code (0: success; negative number: error)
//
//  Programmer:  jodyxha
//  Creation:    Wed Oct 6 11:02:10 PDT 2021
//
// ****************************************************************************

int 
BarGeometry::createBars(int iNumValues, const double *pValues) {
    int iResult = 0;
    double hMax = 0;

    for (uint i = 0; (iResult == 0) && (i < iNumValues); i++) {
        if (pValues[i] > hMax) {
            hMax = pValues[i];
        }
        if (pValues[i] < 0) {
            printf("negative values are not alloewd\n");
            iResult = -1;
        }
    }

   
    double fBarWidth = m_fScaleX/iNumValues;

    double prevX = 0;
    for (uint i = 0; i < iNumValues; i++) {
        double fBarHeight = 0;
        if (hMax > 0) {
            fBarHeight = m_fScaleY * pValues[i]/hMax;
        } else {
            fBarHeight = m_fScaleY;
        }
    
        // draw bar  #i
        m_newPoints->InsertNextPoint(prevX,             0,          0);
        m_newPoints->InsertNextPoint(prevX + fBarWidth, 0,          0);
        m_newPoints->InsertNextPoint(prevX,             fBarHeight, 0);
        
        m_newPoints->InsertNextPoint(prevX + fBarWidth, 0,          0);
        m_newPoints->InsertNextPoint(prevX + fBarWidth, fBarHeight, 0);
        m_newPoints->InsertNextPoint(prevX,             fBarHeight, 0);
        
        const double *curCol = (hMax > 0)?m_vColors[i]:white_color;;
        for (uint k = 0; k < 6; k++) {
            m_newCols->InsertNextTuple(curCol);
        }
        

        // draw bordertop #i
        m_newPoints->InsertNextPoint(prevX,             fBarHeight, 0);
        m_newPoints->InsertNextPoint(prevX + fBarWidth, fBarHeight, 0);
        m_newPoints->InsertNextPoint(prevX,             m_fScaleY + m_fBorder, 0);
     
        m_newPoints->InsertNextPoint(prevX + fBarWidth, fBarHeight, 0);
        m_newPoints->InsertNextPoint(prevX + fBarWidth, m_fScaleY + m_fBorder, 0);
        m_newPoints->InsertNextPoint(prevX,             m_fScaleY + m_fBorder, 0);

        for (uint k = 0; k < 6; k++) {
            m_newCols->InsertNextTuple(black_color);
        }

        prevX += fBarWidth;
    }
    return iResult;
}


// ****************************************************************************
//  Method:  BarGeometry::createSideBorders
//
//  Purpose: 
//    (auxiliary method)
//    calculates the points and triangles for a vertical rectangle at a
//    particular offset, with width=m_fBorder and height=m_fScaleY+2*m_fBorder
//
//  Arguments:
//      fXOff       x offset of rectangeöiNumValues   number of values
//      pValues      array of values
//
//  Returns:     result code (0: success; negative number: error)
//
//  Programmer:  jodyxha
//  Creation:    Wed Oct 6 11:02:10 PDT 2021
//
//----------------------------------------------------------------------------

void 
BarGeometry::createSideBorders(double fXOff) 
{
    m_newPoints->InsertNextPoint(-m_fBorder + fXOff, -m_fBorder,            0);    
    m_newPoints->InsertNextPoint(0 + fXOff,          -m_fBorder,            0);
    m_newPoints->InsertNextPoint(0 + fXOff,           m_fScaleY +m_fBorder, 0);

    m_newPoints->InsertNextPoint(-m_fBorder + fXOff, -m_fBorder,             0);
    m_newPoints->InsertNextPoint(0 + fXOff,           m_fScaleY + m_fBorder, 0);
    m_newPoints->InsertNextPoint(-m_fBorder + fXOff,  m_fScaleY + m_fBorder, 0);

    for (uint k = 0; k < 6; k++) {
        m_newCols->InsertNextTuple(black_color);
    }
}


// ****************************************************************************
//  Method:  BarGeometry::createBottomBorders
//
//  Purpose: 
//    (auxiliary method)
//    calculates the points and triangles for a horizontal rectangle, 
//    with width=m_fScaleX and height=m_fBorder
//
//  Arguments:
//      fXOff       x offset of rectangeöiNumValues   number of values
//      pValues      array of values
//
//  Returns:     result code (0: success; negative number: error)
//
//  Programmer:  jodyxha
//  Creation:    Wed Oct 6 11:02:10 PDT 2021
//
//----------------------------------------------------------------------------
void 
BarGeometry::createBottomBorders() 
{

    // draw bottom border
    m_newPoints->InsertNextPoint(        0, -m_fBorder, 0);
    m_newPoints->InsertNextPoint(m_fScaleX, -m_fBorder, 0);
    m_newPoints->InsertNextPoint(m_fScaleX,          0, 0);
    
    m_newPoints->InsertNextPoint(        0, -m_fBorder, 0);
    m_newPoints->InsertNextPoint(m_fScaleX,          0, 0);
    m_newPoints->InsertNextPoint(        0,          0, 0);
     
    for (uint k = 0; k < 6; k++) {
        m_newCols->InsertNextTuple(black_color);
    }

}
