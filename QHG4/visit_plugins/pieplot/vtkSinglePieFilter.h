// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ****************************************************************************
//  File: vtkSinglePieFilter.h
// ****************************************************************************

#ifndef __VTKSINGLEPIEFILTER_H__
#define __VTKSINGLEPIEFILTER_H__

#include <vtkTransformFilter.h>
#include <vtkPolyDataAlgorithm.h>

#include "GlyphManager.h"

// ********************************************************
// Class: vtkSinglePieFilter
//
// Purpose:
//   A filter handling the data of a single glyph:
//   creates the glyph's transform which is applied to the
//   glyph's vtkPolyData
//
// Programmer: jodyxha
// Creation: August 29, 2021
//
// Modifications:
//
// ******************************

class vtkSinglePieFilter :  public vtkPolyDataAlgorithm 
{
public:
    vtkSinglePieFilter(int iIndex, GlyphManager *pGM);
    virtual ~vtkSinglePieFilter();
    
    //    void SetSourceData(int dom, vtkDataSet *source, vtkTransform *pTransform);
    void SetPieData(double *pPos, double *pNorm, double *pScale, int iNumVals, double *pVals);
    void setGlyphType(int iNewType);
    void setScales(float fScale1, float fScale2);
protected:

    //  virtual int RequestUpdateExtent(vtkInformation *,  vtkInformationVector **, vtkInformationVector *) override;
    virtual int RequestData(vtkInformation *, vtkInformationVector **, vtkInformationVector *) override;
    void cleanArrays();

    int                 m_iIndex;

    vtkTransformFilter *m_pTransformFilter;
    vtkDataSet         *m_pSource;
    GlyphManager       *m_pGM;
   
    int m_iGlyphType;
};

#endif

