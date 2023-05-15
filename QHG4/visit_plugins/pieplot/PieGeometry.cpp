// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ****************************************************************************
//  File: PieGeometry.C
// ****************************************************************************

#include <vtkMath.h>
#include <vtkCellArray.h>
#include <vtkFloatArray.h>
#include <vtkObjectFactory.h>
#include <vtkPointData.h>
#include <vtkCellData.h>
#include <vtkPoints.h>
#include <vtkPolyData.h>
#include <vtkActor.h>
#include <vtkTriangle.h>
#include <vtkPolyDataMapper.h>
#include <vtkUnsignedCharArray.h>
#include <vtkTransform.h>
#include <vtkTransformFilter.h>

#include <DebugStream.h>

#include "PieGeometry.h"
#include "GlyphGeometry.h"
#include "GlyphColors.h"

#define NUMSECTORS 200
#define RAD1       1.0
#define RAD2       1.1



// ****************************************************************************
//  Method: PieGeometry constructor
//
//  Arguments:
//      iNumSectors  number of sectors to use (more sectors -> rounder pie)
//      dR1          radius of pie
//      dR2          outer radius of pie (pie + border)
//      colvec       array with colors for the values
//
//  Programmer: jodyxha
//  Creation:   Wed Oct 6 11:02:10 PDT 2021
//
// ****************************************************************************

PieGeometry::PieGeometry(int iNumSectors, double dR1, double dR2, colvec &vColors)
    : GlyphGeometry("Pie", vColors),
      m_iNumSectors(iNumSectors),
      m_dR1(dR1),
      m_dR2(dR2) 
{
}


// ****************************************************************************
//  Method: PieGometry destructor
//
//  Programmer: jodyxha
//  Creation:   Wed Oct 6 11:02:10 PDT 2021
//
// ****************************************************************************

PieGeometry::~PieGeometry() {
}
 

// ****************************************************************************
//  Method:  PieGeometry::createGlyph
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
//----------------------------------------------------------------------------

int
PieGeometry::createGlyph(int iNumValues, const double *pValues) 
{
    
    int iResult = 0;
    debug1 << "QHG:[PieGeometry::createGlyph] Building a glyph (with " << iNumValues << " values)" << endl << std::flush;

    if (iNumValues == 0) {
        iResult = createBlankPie();
    } else {
        iResult = createRegularPieNew(iNumValues, pValues);
    }

    return iResult;
}

// ****************************************************************************
//  Method:  PieGeometry::createBlankPie
//
//  Purpose: 
//    calculate the points and triangles for a full white circle.
//    This is used to display an empty histogram
//
//  Returns:     result code (0: success; negative number: error)
//
//  Programmer:  jodyxha
//  Creation:    Tue Apr 4 14:58:10 PDT 2023
//
//----------------------------------------------------------------------------

int
PieGeometry::createBlankPie() 
{
    int iResult = 0;
    uint iNumTriangles  = m_iNumSectors * 3;
    uint iNumVerts      = iNumTriangles * 3;
 
    double delta = 2*M_PI/m_iNumSectors;

    debug1 << "QHG:[PieGeometry::createBlankPie] Building a blank pie" << endl << std::flush;
    double alpha_cur = 0;
    double alpha_end = 0.0;
    for (uint i = 0; i < m_iNumSectors; i++) {
        createPieSegment(alpha_cur, alpha_cur+delta, white_color);
        alpha_cur += delta;
    }
    return iResult;
}

// ****************************************************************************
//  function:  findMaxIndex
//
template<typename T>
int findMaxIndex(int iNumValues, T *pPart) {
    int k = -1;
    T iMaxVal = INT_MIN; 
    for (int i = 0; i < iNumValues; i++) {
        if (pPart[i] > iMaxVal) {
            iMaxVal = pPart[i];
            k = i;
        }
    }
    return k;
}

// ****************************************************************************
//  function:    makePartition
//
// ****************************************************************************
//  Method:  PieGeometry::makePartition
//
//  Purpose: 
//    create a partition which comes close to n_i = p_i*N
//
//  Arguments:
//      iNumValues   number of values
//      pValues      array of values
//      fSum         sum of values
//
//  Returns:     result code (0: success; negative number: error)
//
//  Programmer:  jodyxha
//  Creation:    Thu Apr 06 2023
//
int *PieGeometry::makePartition(int iNumValues, const double *pValues, double fSum) {

    /*
    debug1 << "QHG:[PieGeometry::makePartition] for N = " << m_iNumSectors << ", numvals "<< iNumValues << endl << std::flush;
    for (int i = 0; i < iNumValues; i++)  {
        debug1 << "QHG:[PieGeometry::makePartition] pValues[" << i << "] = "<< pValues[i] << endl << std::flush;
    }
    */

    int *pPartition = new int [iNumValues];
    memset(pPartition, 0, iNumValues*sizeof(int));
    int *pCurPartition = new int [iNumValues];
    int N = m_iNumSectors;
    while (N > 0) {
        memset(pCurPartition, 0, iNumValues*sizeof(int));
        int iS = 0;
        
        for (int i = 0; i < iNumValues; i++)  {
            pCurPartition[i] = round(N * pValues[i]/fSum);
            iS += pCurPartition[i];
        }

        if (iS <= N) {
            if (iS == 0) {
                int k = findMaxIndex(iNumValues, pCurPartition);
                pCurPartition[k]++;
                iS = 1;
            }
            N = N - iS;
        } else {
            int r = iS - N;
            while (r > 0) {
                int k = findMaxIndex(iNumValues, pCurPartition);
                pCurPartition[k]--;
                r--;

            }
            N = 0;
        }
        
        for (int i = 0; i < iNumValues; i++) {
            pPartition[i] += pCurPartition[i];
        }

    }

    return pPartition;
} 

// ****************************************************************************
//  Method:  PieGeometry::createRegularPieNew
//
//  Purpose: 
//    calculate the points and triangles for regular pie glyphs
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
//----------------------------------------------------------------------------
int
PieGeometry::createRegularPieNew(int iNumValues, const double *pValues) 
{
    
    int iResult = 0;

    uint iNumTriangles  = m_iNumSectors  * 3;
    uint iNumVerts      = iNumTriangles * 3;

    // allocate points
    m_newPoints->SetDataType(VTK_DOUBLE);
    m_newPoints->Allocate(iNumVerts);
    

    debug1 << "QHG:[PieGeometry::createRegularPieNew] Building a regular pie" << endl << std::flush;
    
    // allocate triangless
    vtkNew<vtkCellArray> newPolys;
    newPolys->Allocate(iNumTriangles);

    // we need the sum of values to calculate the angles of the pie slices
    double fSum = 0;
    for (uint i = 0; (iResult == 0) && (i < iNumValues); i++) {
        if (pValues[i] >= 0) {
            fSum += pValues[i];
        } else {
    debug1 << "QHG:[PieGeometry::createRegularPieNew] negative values not allowed " << endl << std::flush;
            printf("negative values are not alloewd\n");
            iResult = -1;
            fSum = 0;
        }
    }

    double delta = 2*M_PI/m_iNumSectors;

    if (fSum > 0) {
        debug1 << "QHG:[PieGeometry::createRegularPieNew] making partition " << endl << std::flush;
        int *pPartition = makePartition(iNumValues, pValues, fSum);
        debug1 << "QHG:[PieGeometry::createRegularPieNew] partition done " << endl << std::flush;
        double alpha_prev = 0;
        for (int i = 0; i < iNumValues; i++) {
            // calc angle corresponding to  pPartition[i] sectors
            //            double alpha_cur = alpha_prev + 2*M_PI*pPartition[i]/m_iNumSectors;
            
            for (int k = 0; k < pPartition[i]; k++) {
                debug1 << "QHG:[PieGeometry::createRegularPinew] drawing color " << i << "; alpha_prev " << alpha_prev << ", alpha_cur "<< alpha_prev+delta << endl << std::flush;
                createPieSegment(alpha_prev, alpha_prev+delta, m_vColors[i]);
                alpha_prev += delta;
            }
           
           
        }
    } else {
        debug1 << "QHG:[PieGeometry::createRegularPieNew] sum is zero: all black" <<  endl << std::flush;
        // all zero: black circle
        double alpha_cur = 0;
        double alpha_end = 0.0;
        for (uint i = 0; i < m_iNumSectors; i++) {
            debug1 << "QHG:[PieGeometry::createRegularPinew] drawing black " << i << "; alpha_cur " << alpha_cur << ", alpha_cur+delta "<< alpha_cur+delta << endl << std::flush;
            createPieSegment(alpha_cur, alpha_cur+delta, white_color);
            alpha_cur += delta;
        }
    }
    
    debug1 << "QHG:[PieGeometry::createRegularPieNew] making triangles" << endl << std::flush;
 
    // make the triangles
    for (uint i = 0; i < iNumVerts; i+=3) {
        vtkTriangle *triangle1 =vtkTriangle::New();
        triangle1->GetPointIds()->SetId(0, i);
        triangle1->GetPointIds()->SetId(1, i+1);
        triangle1->GetPointIds()->SetId(2, i+2);
        newPolys->InsertNextCell(triangle1);
    }

    debug1 << "QHG:[PieGeometry::createRegularPieNew] making polydara" << endl << std::flush;
    m_newPolyData = vtkPolyData::New();
    
    m_newPolyData->SetPoints(m_newPoints);
    m_newPolyData->GetPointData()->SetScalars(m_newCols);
    m_newPolyData->SetPolys(newPolys);  

    debug1 << "QHG:[PieGeometry::createRegularPieNew] done" << endl << std::flush;
    
    return iResult;
}



// ****************************************************************************
//  Method:  PieGeometry::createPieSegment
//
//  Purpose: 
//    auxiliary method to calculate the points and triangles of a single segment
//
//  Arguments:
//      alpha_prev  starting angle of segment
//      alpha_cur   end angle of segment 
//      curCol      color for the segment
//  
//  Programmer: jodyxha
//  Creation:    Wed Oct 6 11:02:10 PDT 2021
//
//----------------------------------------------------------------------------

void 
PieGeometry::createPieSegment(double alpha_prev, double alpha_cur, const double *curCol) 
{
    if (alpha_cur > alpha_prev) {   
    // sector
    m_newPoints->InsertNextPoint(0, 0, 0);
    m_newPoints->InsertNextPoint(m_dR1*cos(alpha_prev), m_dR1*sin(alpha_prev), 0);
    m_newPoints->InsertNextPoint(m_dR1*cos(alpha_cur),  m_dR1*sin(alpha_cur),  0);

    m_newCols->InsertNextTuple(curCol);
    m_newCols->InsertNextTuple(curCol);
    m_newCols->InsertNextTuple(curCol);

    // border 1
    m_newPoints->InsertNextPoint(m_dR1*cos(alpha_prev), m_dR1*sin(alpha_prev), 0);
    m_newPoints->InsertNextPoint(m_dR2*cos(alpha_cur),  m_dR2*sin(alpha_cur),  0);
    m_newPoints->InsertNextPoint(m_dR1*cos(alpha_cur),  m_dR1*sin(alpha_cur),  0);

    m_newCols->InsertNextTuple(black_color);
    m_newCols->InsertNextTuple(black_color);
    m_newCols->InsertNextTuple(black_color);


    // border 2
    m_newPoints->InsertNextPoint(m_dR1*cos(alpha_prev), m_dR1*sin(alpha_prev), 0);
    m_newPoints->InsertNextPoint(m_dR2*cos(alpha_prev), m_dR2*sin(alpha_prev), 0);
    m_newPoints->InsertNextPoint(m_dR2*cos(alpha_cur),  m_dR2*sin(alpha_cur),  0);

    m_newCols->InsertNextTuple(black_color);
    m_newCols->InsertNextTuple(black_color);
    m_newCols->InsertNextTuple(black_color);
    }
}

