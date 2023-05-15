// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//  File: avtPieMapper.h
// ************************************************************************* //


//@@ following avtTensorGlyphMapper

#ifndef AVT_PIE_MAPPER_H
#define AVT_PIE_MAPPER_H

#include <avtMapper.h>
#include <vector>
#include <string>

#include <PieAttributes.h>
#include <vtkType.h>
#include <vtkDataSetMapper.h>

#include "vtkSinglePieFilter.h"
#include "GlyphManager.h"

// ****************************************************************************
//  Class: avtPieMapper
//
//  Purpose:
//    Pie plot specific mapper.
//
//  Programmer: jody
//  Creation:   Oct 07, 2021
//
//  Modifications:
//
// ****************************************************************************

class avtPieMapper : public avtMapper
{
  public:
                               avtPieMapper(GlyphManager *pGM);
    virtual                   ~avtPieMapper();

    void setGlyphType(int iNewType);
   
    const std::vector<std::string> getComponentNames() { return m_vComponentNames;};
    int getNumValues() {return m_vComponentNames.size();};
    std::string getFamily() { return m_sFamily; };

    static void showTree(avtDataTree_p pTree);
 
    void SetColors(colvec &vColors);
  protected:
    virtual vtkDataSetMapper  *CreateMapper(void);
    virtual void               CustomizeMappers(void);

    virtual vtkAlgorithmOutput *InsertFilters(vtkDataSet *, int);
    virtual void               SetUpFilters(int);

    void extractComponentNames();

    int                        m_nSinglePieFilters;
    vtkSinglePieFilter       **m_ppSinglePieFilter;

    GlyphManager *m_pGM;
    int m_iGlyphType;
    std::vector<std::string>   m_vComponentNames;
    std::string                m_sFamily;
    colvec                     m_vColors;

};

#endif

