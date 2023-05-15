// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ****************************************************************************
//  File: vtkSinglePieFilter.C
// ****************************************************************************

#include <vtkInformation.h>
#include <vtkInformationVector.h>
#include <vtkPointData.h>

#include <DebugStream.h>
#include "vtkSinglePieFilter.h"


// ****************************************************************************
//  Method: vtkSinglePieFilter constructor
//
//  Arguments:
//      iIndex       index of glyph to draw
//      pG           GlyphManager holding the glyphs
//
//  Programmer: jodyxha
//  Creation:   Wed Oct 6 11:02:10 PDT 2021
//
// ****************************************************************************

vtkSinglePieFilter::vtkSinglePieFilter(int iIndex, GlyphManager *pGM) 
    : m_iIndex(iIndex),
      m_pTransformFilter(NULL),
      m_pGM(pGM),
      m_iGlyphType(0) 
{
    //    debug1 << "QHG: [vtkSinglePieFilter::vtkSinglePieFilter]" << endl << std::flush;
}


// ****************************************************************************
//  Method: vtkSinglePieFilter destructor
//
//  Programmer: jodyxha
//  Creation:   Wed Oct 6 11:02:10 PDT 2021
//
// ****************************************************************************

vtkSinglePieFilter::~vtkSinglePieFilter() 
{
    if (m_pTransformFilter != NULL) { 
        m_pTransformFilter->Delete();
    }

}


// ****************************************************************************
//  Method:  vtkSinglePieFilter::setGlyphType
//
//  Purpose: 
//   sets the glyph type to use for drawing
//
//  Arguments:
//      iNewType    glyph type
//
//  Programmer:  jodyxha
//  Creation:    Wed Oct 6 11:02:10 PDT 2021
//
// ****************************************************************************


void vtkSinglePieFilter::setGlyphType(int iNewType) {
    //    debug1 << "QHG: [vtkSinglePieFilter::setGlyphType(" << iNewType << ")]" << endl << std::flush;
    m_iGlyphType = iNewType;
}


// ****************************************************************************
//  Method:  vtkSinglePieFilter::RequestData
//
//  Purpose: 
//    sets  points and triangles to the output information
//
//  Arguments:
//      vtkNotUsed      ???
//      inputVector     input data information
//      outputVector    output data information
//
//  Returns:     result code (0: success; negative number: error)
//
//  Programmer:  jodyxha
//  Creation:    Wed Oct 6 11:02:10 PDT 2021
//
// ****************************************************************************

int 
vtkSinglePieFilter::RequestData(vtkInformation *vtkNotUsed(request), vtkInformationVector **inputVector, vtkInformationVector *outputVector) 
{
    //    debug1 << "QHG: [vtkSinglePieFilter::RequestData]" << endl << std::flush;
    //    
    
    // get the info objects
    vtkInformation *sourceInfo = inputVector[0]->GetInformationObject(0);
    vtkInformation *outInfo = outputVector->GetInformationObject(0);
    
    // get the input and output
    vtkPolyData *source = vtkPolyData::SafeDownCast(sourceInfo->Get(vtkDataObject::DATA_OBJECT()));
    vtkPolyData *output = vtkPolyData::SafeDownCast(outInfo->Get(vtkDataObject::DATA_OBJECT()));
 
  
    // forward the points without change (they are already transformed)
    vtkPoints *sourcePts = source->GetPoints();
    output->SetPoints(sourcePts); 

    // forward the cells without change
    vtkCellArray *sourceCells = source->GetPolys();
    output->SetPolys(source->GetPolys());

    // forward the colors
    output->GetPointData()->SetScalars(source->GetPointData()->GetScalars());
        
    return 1;
}


// ****************************************************************************
//  Method:  vtkSinglePieFilter::SetPieData
//
//  Purpose: 
//    here we create the vtkPolyData for theglyph, create the transformation 
//    and a vtkTransformFilter to hold it.
//    Hook the transform filter's output to this vtkSinglePieFilter's input
//    (that's how we get transformed data in RequestData())
//
//  Arguments:
//      pPos       position of glyph (defines the translation)
//      pNorm      orientation of glyph (defines rotation)
//      pScale     scaling of glyph 
//      iNumVals   number of values in glyph
//      pVals      values for the glyph
//
//  Returns:     result code (0: success; negative number: error)
//
//  Programmer:  jodyxha
//  Creation:    Wed Oct 6 11:02:10 PDT 2021
//
// ****************************************************************************

void vtkSinglePieFilter::SetPieData(double *pPos, double *pNorm, double *pScale, int iNumVals, double *pVals) 
{
    vtkPolyData *pPD = m_pGM->createGlyph(m_iGlyphType, pPos, pNorm, pScale[0], iNumVals, pVals);
    //    debug1 << "QHG: [vtkSinglePieFilter::SetPieData] ("<<this<<") got index " << m_iIndex  << endl << std::flush;

    vtkTransform *pTransform = GlyphManager::calcTransform(pPos, pNorm, pScale, true); // true: add transformation to make glyphs upright
    m_pTransformFilter = vtkTransformFilter::New();
    m_pTransformFilter->SetInputData(pPD);
    m_pTransformFilter->SetTransform(pTransform);
    m_pTransformFilter->Update();

    SetInputConnection(m_pTransformFilter->GetOutputPort());
}
