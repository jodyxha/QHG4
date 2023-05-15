// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ****************************************************************************
//  File: BoxGeometry.C
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

#include "BoxGeometry.h"
#include "GlyphGeometry.h"
#include "GlyphColors.h"

#define SCALE_X   1.0
#define SCALE_Y   1.0
#define BORDER    0.1


// ****************************************************************************
//  Method: BoxGeometry constructor
//
//  Arguments:
//      fScaleX     width of box strip
//      fScaleY     height of box strip
//      fBorder     border width
//      colvec      array with colors for the values
//
//  Programmer: jodyxha
//  Creation:   Wed Oct 6 11:02:10 PDT 2021
//
// ****************************************************************************
BoxGeometry::BoxGeometry(double fScaleX, double fScaleY, double fBorder, colvec &vColors)
    : GlyphGeometry("Box", vColors),
      m_fScaleX(fScaleX),
      m_fScaleY(fScaleY),
      m_fBorder(fBorder) 
{

}


// ****************************************************************************
//  Method: BoxGometry destructor
//
//  Programmer: jodyxha
//  Creation:   Wed Oct 6 11:02:10 PDT 2021
//
// ****************************************************************************
BoxGeometry::~BoxGeometry() 
{
}

 
// ****************************************************************************
//  Method:  BoxGeometry::createGlyph
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
BoxGeometry::createGlyph(int iNumValues, const double *pValues) 
{
    int iResult = 0;
   
    uint iNumTriangles =8 + 2 * iNumValues;
    uint iNumVerts     = 3*iNumTriangles;

    m_newCols->SetNumberOfComponents(3);
    m_newCols->SetName("Colors");
    
    m_newPoints->SetDataType(VTK_DOUBLE);
    m_newPoints->Allocate(iNumVerts);
    
    vtkNew<vtkCellArray> newPolys;
    newPolys->Allocate(iNumTriangles);

    // the boxes themselves 
    iResult = createValueBoxes(iNumValues, pValues);
  
    // the borders
    createVertBorders(0);
    createVertBorders(m_fScaleX + m_fBorder);
    createHoriBorders(0);
    createHoriBorders(m_fScaleY + m_fBorder);
    
    // shift the center to 0
    for (uint i = 0; i < iNumVerts; i++) {
        double *fx = m_newPoints->GetPoint(i);
        fx[0] -= m_fScaleX/2;
        fx[1] -= m_fScaleY/2;
        m_newPoints->SetPoint(i, fx);
    }

    // define the triangles
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
    
    m_newPolyData->SetPolys(newPolys);  

    return iResult;
}


// ****************************************************************************
//  Method:  BarGeometry::drawBox
//
//  Purpose: 
//    calculate the points and triangles for an axis-parallel box given 
//    xmin,ymin and xmax,ymax and sets the color
//
//  Arguments:
//      iXMin    x coordinate left side
//      iYMin    y coordinate bottom side
//      iXMax    x coordinate right side
//      iYMax    y coordinate top side
//      aCurCol  the color to be usead
//
//  Programmer:  jodyxha
//  Creation:    Wed Oct 6 11:02:10 PDT 2021
//
// ****************************************************************************
void  
BoxGeometry::drawBox(double iXMin, double iYMin,double iXMax, double iYMax, const double *aCurCol) 
{
    // draw the first triangle  iXMin,iYMin - iXMax,iYMin - iXMax,iYMax
    m_newPoints->InsertNextPoint(iXMin, iYMin, 0.0);
    m_newPoints->InsertNextPoint(iXMax, iYMin, 0.0);
    m_newPoints->InsertNextPoint(iXMax, iYMax, 0.0);

    // draw the second triangle  iXMin,iYMin - iXMax,iYMax - iXMin,iYMax
    m_newPoints->InsertNextPoint(iXMin, iYMin, 0.0);
    m_newPoints->InsertNextPoint(iXMax, iYMax, 0.0);
    m_newPoints->InsertNextPoint(iXMin, iYMax, 0.0);

    for (uint k = 0; k < 6; k++) {
        m_newCols->InsertNextTuple(aCurCol);
    }
}


// ****************************************************************************
//  Method:  BarGeometry::createValueBoxes
//
//  Purpose: 
//    calculate the boxes for all values
//
//  Arguments:
//      iNumValues   number of values
//      pValues      array of values
//
//  Returns      result code (-1 if there is a negative value)
//  Programmer:  jodyxha
//  Creation:    Wed Oct 6 11:02:10 PDT 2021
//
// ****************************************************************************
int 
BoxGeometry::createValueBoxes(int iNumValues, const double *pValues) 
{
    int iResult = 0;
    double hMax = 0;

    double fSum = 0;
    for (uint i = 0; (iResult == 0) && (i < iNumValues); i++) {
        if (pValues[i] >= 0) {
            fSum += pValues[i];
        } else {
            printf("negative values are not alloewd\n");
            iResult = -1;
            fSum = 0;
        }
    }

    if (fSum > 0) {
        double prevX = 0;
        for (uint i = 0; i < iNumValues; i++) {
            drawBox(prevX, 0, prevX+m_fScaleX*pValues[i]/fSum, m_fScaleY, m_vColors[i]);
            prevX += m_fScaleX*pValues[i]/fSum;
        } 
    }else {
        drawBox(0, 0, m_fScaleX, m_fScaleY, white_color);
    }
    return iResult;
}


// ****************************************************************************
//  Method:  BarGeometry::createVertBorders
//
//  Purpose: 
//     create a black vertical border rectangle with width m_fBorder and
//     height m_fScaleY+2*m_fBoorderto the left of fXOff
//
//  Arguments:
//      xOff     x iNumValues   number of values
//      pValues      array of values
//
//  Programmer:  jodyxha
//  Creation:    Wed Oct 6 11:02:10 PDT 2021
//
// ****************************************************************************
void 
BoxGeometry::createVertBorders(double fXOff) 
{
    drawBox(-m_fBorder + fXOff,  -m_fBorder, fXOff,  m_fScaleY + m_fBorder, black_color); 
}


// ****************************************************************************
//  Method:  BarGeometry::createHoriBorders
//
//  Purpose: 
//     create a black horizonzal border rectangle with height m_fBorder and 
//     height m_fScaleX starting at 0,0
//
//  Arguments:
//      xOff     x iNumValues   number of values
//      pValues      array of values
//
//  Programmer:  jodyxha
//  Creation:    Wed Oct 6 11:02:10 PDT 2021
//
// ****************************************************************************
void 
BoxGeometry::createHoriBorders(double fYOff) 
{
    drawBox(0.0, -m_fBorder + fYOff, m_fScaleX, fYOff, black_color);

}
