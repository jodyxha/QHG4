// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ****************************************************************************
//  File: avtPieMapper.C
// ****************************************************************************

#include <sstream>
#include <vtkPointData.h>
#include <vtkStringArray.h>
#include <DebugStream.h>

#include "avtPieMapper.h"
#include "GlyphColors.h"
#include "GlyphManager.h"
#include "vtkSinglePieFilter.h"

//----------------------------------------------------------------------------
// showTree
//  show the contents of a tree on debug1
//
// ****************************************************************************
//  Method:  avtPieMapper::showTree
//
//  Purpose: 
//    (auxiliary method) lists the contents of a data tree
//
//  Arguments:
//      pTree           the data tree to be inspected
//
//  Programmer:  jodyxha
//  Creation:    Wed Oct 6 11:02:10 PDT 2021
//
// ****************************************************************************

void avtPieMapper::showTree(avtDataTree_p pTree) {
    int iNumArrays = 0;

    vtkDataSet **ppArrays = pTree->GetAllLeaves(iNumArrays);
    debug1 << "QHG: [avtPieMapper::SetupFilters] got " << iNumArrays << " " <<((iNumArrays == 1)?"Leaf":" Leaves") << endl << std::flush;
    for (int i = 0; i < iNumArrays; i++) {
        vtkDataSet *curDS = ppArrays[i];
        debug1 << "QHG: [avtPieMapper::SetupFilters] curDS: " << curDS << endl << std::flush;
        
        // print the entire dataset
        std::ostringstream os;
        curDS->Print(os);
        debug1 << os.str() << endl << std::flush;
        os.str("yadayada");
        /*
        // display the PointData
        vtkPointData *pPointData = curDS->GetPointData();
        debug1 << "QHG: [avtPieMapper::SetupFilters] pointdata: " << pPointData << endl << std::flush;
        // print the pointdata        
        pPointData->Print(os);
        debug1 << os.str() << endl << std::flush;
        os.flush();
        */   
        /*
        // display format of first array in pointData
        vtkDataArray *pDataArray = pPointData->GetArray(0);
        debug1 << "QHG: [avtPieMapper::SetupFilters] datarray: " << pDataArray << endl << std::flush;

        int iNumTuples = pDataArray->GetNumberOfTuples();
        int iNumComp = pDataArray->GetNumberOfComponents();
    
        debug1 << "QHG: [avtPieMapper::SetupFilters]  Leaf " << i << "(PD): " << iNumTuples << " tuples.  " << iNumComp << " components" << endl << std::flush;
        */
        // show field data contents (expect some numeric arrays(pos,born,scale,vals) and a string array (val names)
        vtkFieldData *pFieldData = curDS->GetFieldData();
        int iNumFArrs = pFieldData->GetNumberOfArrays();
        debug1 << "QHG: [avtPieMapper::SetupFilters]  Leaf " << i << "(FD): " << iNumFArrs << " arrays" << endl << std::flush;
        for (int k = 0; k < iNumFArrs; k++) {
            vtkAbstractArray *pADataArray = pFieldData->GetAbstractArray(k);
            if (pADataArray->IsNumeric()) {
                vtkDataArray *pFDataArray = vtkDataArray::SafeDownCast(pADataArray);
                
                debug1 << "QHG: [avtPieMapper::SetupFilters]  Got array " << k << " (" << pFieldData->GetArrayName(k) << ")" <<endl <<  std::flush;
                
                int iNumFTuples = pFDataArray->GetNumberOfTuples();
                //debug1 << "QHG: [avtPieMapper::SetupFilters]  Got number of tuples " << iNumFTuples << endl <<  std::flush;
                debug1 << "QHG: [avtPieMapper::SetupFilters]  Got number of tuples " << endl <<  std::flush;
                
                for (int h = 0; h < iNumFTuples; h++) {
                    double *pContents =  pFDataArray->GetTuple(h);
                    debug1 << "QHG: [avtPieMapper::SetupFilters]  Got tuple " << h << " of " << iNumFTuples << " in array " << k <<endl <<  std::flush;
                    int iNumFComp = pFDataArray->GetNumberOfComponents();
                    debug1 << "QHG: [avtPieMapper::SetupFilters]  Leaf " << i << "(FD) arr#" << k <<" ('" << pFieldData->GetArrayName(k) << "'): "  << iNumFTuples << " tuples,  " << iNumFComp << " components:";
                    debug1 << "  ";
                    for (int j = 0; j <  pFDataArray->GetNumberOfComponents(); j++) {
                        debug1 << " " << pContents[j] << std::flush;
                    }
                }
                debug1 << endl << std::flush;
            } else {
                if (pADataArray->GetDataType() == VTK_STRING) {
                    vtkStringArray *pSA = vtkStringArray::SafeDownCast(pADataArray);
                    debug1 << "QHG: [avtPieMapper::SetupFilters] as string array: " << pSA << endl << std::flush;
                    int inv =  pSA->GetNumberOfValues();
                    debug1 << "QHG: [avtPieMapper::SetupFilters] number of values: "<< inv << endl  << std::flush;
                    for (int i  = 0; i < inv; i++) {
                        debug1 << "  " << pSA->GetValue(i) << endl <<std::flush;
                    }
                } else {
                    debug1 << "not string or numeric; datatype = " << pADataArray->GetDataType() << endl << std::flush;
                }
            }
                    
            //    debug1 << endl << std::flush;
                
                // }
        }
        // if we're looking at a correct data tree, we can look at the contents of the arrays
        /*
        vtkDataArray *pDataArrayPos   = pFieldData->GetArray(0);
        vtkDataArray *pDataArrayNorm  = pFieldData->GetArray(1);
        vtkDataArray *pDataArrayScale = pFieldData->GetArray(2);
        vtkDataArray *pDataArrayVals  = pFieldData->GetArray(3);
        double *pPos    = pDataArrayPos->GetTuple(0);
        double *pNorm   = pDataArrayNorm->GetTuple(0);
        double *pScale  = pDataArrayScale->GetTuple(0);
        double *pVals   = pDataArrayVals->GetTuple(0);
        debug1 << "QHG: [avtPieMapper::SetupFilters]  point (" <<  pDataArrayPos->GetNumberOfComponents() << "): " << pPos[0] << " " << pPos[1] << " " <<  pPos[2] << endl << std::flush;
        debug1 << "QHG: [avtPieMapper::SetupFilters]  norm  (" <<  pDataArrayNorm->GetNumberOfComponents() << "): " << pNorm[0]  << " " << pNorm[1]  << " " <<  pNorm[2]  << endl << std::flush;
        debug1 << "QHG: [avtPieMapper::SetupFilters]  vals  (" <<  pDataArrayVals->GetNumberOfComponents() << "): ";
        for (int j = 0; j <  pDataArrayVals->GetNumberOfComponents(); j++) {
            debug1 << " " << pVals[j] << std::flush;
        }
        debug1 << endl << std::flush;
        */
    }
}


// ****************************************************************************
//  Method: PieMapper::avtPieMapper
//
//  Purpose:
//    Constructor for the avtPieMapper class.
//
//  Programmer: jodyxha
//  Creation:   Oct 07, 2021
//
//  Modifications:
//
// ****************************************************************************

avtPieMapper::avtPieMapper(GlyphManager *pGM)
    : m_nSinglePieFilters(0),
      m_ppSinglePieFilter(NULL),
      m_pGM(pGM),
      m_iGlyphType(0) {
    debug1 << "QHG: [avtPieMapper::avtPieMapper()] (" << this << ")" << endl <<  std::flush;

    
    // we need a non-empty color vector
    for (int i = 0; i < sizeof(basic_colors)/(3*sizeof(double)); i++) {
        m_vColors.push_back(basic_colors[i]);
    }
    m_pGM->setGlyphColors(m_vColors);
    
}


// ****************************************************************************
//  Method: avtPieMapper::~avtPieMapper
//
//  Purpose: 
//    Destructor for the avtPieMapper class.
//
//  Programmer: jodyxha
//  Creation:   Oct 07, 2021
//
//  Modifications:
//
// ****************************************************************************

avtPieMapper::~avtPieMapper() {
    debug1 << "QHG: [avtPieMapper::~avtPieMapper()] (" << this << "), family [" << m_sFamily << "]" << endl <<  std::flush;
    if (m_ppSinglePieFilter != NULL) {
        for (int i = 0 ; i < m_nSinglePieFilters ; i++) {
            if (m_ppSinglePieFilter[i] != NULL) {
                //delete m_ppSinglePieFilter[i];
                m_ppSinglePieFilter[i]->Delete();
            }
        }
        delete [] m_ppSinglePieFilter;
    }
    debug1 << "QHG: [avtPieMapper::~avtPieMapper()] exit" << endl <<  std::flush;
}


// ****************************************************************************
//  Method: avtPieMapper::setGlyphType
//
//  Purpose: 
//    Pass the new GylphType to all vtkSinglePieFilters
//
//  Programmer: jodyxha
//  Creation:   Oct 07, 2021
//
//  Modifications:
//
// ****************************************************************************
void 
avtPieMapper::setGlyphType(int iNewType) {
    debug1 << "QHG: [avtPieMapper::setGlyphType(" << iNewType << ")] exit" << endl <<  std::flush;
    m_iGlyphType = iNewType;

    // this method can be called before the SinglePieFilters are created
    // so they won't get the new GlyphType (see InserFilters())
    if (m_ppSinglePieFilter != NULL) {
        for (int i = 0 ; i < m_nSinglePieFilters ; i++) {
            if (m_ppSinglePieFilter[i] != NULL) {
                m_ppSinglePieFilter[i]->setGlyphType(m_iGlyphType);
            }
        }
    }

}


// ****************************************************************************
//  Method: avtPieMapper::CreateMapper
//
//  Purpose:
//    Creates a vtkPieMapper.
//
//  Returns:    A pointer to the new mapper.
//
//  Programmer: jodyxha
//  Creation:   Oct 07, 2021
//
//  Modifications:
//
// ****************************************************************************

vtkDataSetMapper *
avtPieMapper::CreateMapper(void)
{
    return vtkDataSetMapper::New();
}


// ****************************************************************************
//  Method: avtPieMapper::CustomizeMappers
//
//  Purpose:
//
//  Programmer: jodyxha
//  Creation:   Oct 07, 2021
//
//  Modifications:
//
// ****************************************************************************

void
avtPieMapper::CustomizeMappers(void)
{
   debug1 << "QHG: [avtPieMapper::CustomizeMappers()]" << endl <<  std::flush;
   // we probably wont't hzave to implement this

    //    showTree(GetInputDataTree());
}


// ****************************************************************************
//  Method: avtPieMapper::SetupFilters
//
//  Purpose:
//    allocate the array of the vtkSinglePieFilters
//
//  Programmer: jodyxha
//  Creation:   Nov 01, 2021
//
//  Modifications:
//
// ****************************************************************************

void
avtPieMapper::SetUpFilters(int nDoms) {

    debug1 << "QHG: [avtPieMapper::SetupFilters(" << nDoms <<")]" << endl <<  std::flush;

    // first some cleaning up
    if (m_ppSinglePieFilter != NULL) {
        for (int i = 0 ; i < m_nSinglePieFilters ; i++) {
            if (m_ppSinglePieFilter[i] != NULL) {
                m_ppSinglePieFilter[i]->Delete();
            }
        }
        delete [] m_ppSinglePieFilter;
    }
    
    // now create new ones; they will be filled in InsertFilters()
    m_nSinglePieFilters     = nDoms; 
    m_ppSinglePieFilter     = new vtkSinglePieFilter*[m_nSinglePieFilters];
    for (int i = 0 ; i < m_nSinglePieFilters ; i++)
    {
        m_ppSinglePieFilter[i] = NULL;
    }

    // extract the  value names
    extractComponentNames();

    debug1 << "QHG: [avtPieMapper::SetupFilters] Component names:" << endl <<std::flush;
    for (uint i = 0; i < m_vComponentNames.size(); i++) {
        debug1 << "QHG: [avtPieMapper::SetupFilters]      " << m_vComponentNames[i]  << endl <<std::flush;
    }
    // showTree(GetInputDataTree());
}



// ****************************************************************************
//  Method: avtPieMapper::InsertFilters
//
//  Purpose:
//    create a vtkSinglePieFilter and provide it with the arrays ectracted from
//    the data set (one of the leaves of the tree created by avtPieFilter)
//
//  Programmer: jodyxha
//  Creation:   Nov 01, 2021
//
//  Modifications:
//
// ****************************************************************************

vtkAlgorithmOutput *
avtPieMapper::InsertFilters(vtkDataSet *ds, int dom)
{
    debug1 << "QHG: [avtPieMapper::InsertFilters(ds, " << dom <<")]" << endl <<  std::flush;

    if (m_ppSinglePieFilter[dom] == NULL) {
        m_ppSinglePieFilter[dom] = new vtkSinglePieFilter(dom, m_pGM);
    }

    // here, the new GlyphType will certainly be known
    m_ppSinglePieFilter[dom]->setGlyphType(m_iGlyphType);
  
    // extract the arrays from the dataset
    vtkFieldData *pFieldData      = ds->GetFieldData();
    vtkDataArray *pDataArrayPos   = pFieldData->GetArray("pos");
    vtkDataArray *pDataArrayNorm  = pFieldData->GetArray("norm");
    vtkDataArray *pDataArrayScale = pFieldData->GetArray("scale");
    vtkDataArray *pDataArrayVals  = pFieldData->GetArray("vals");
    double *pPos   = pDataArrayPos->GetTuple(0);
    double *pNorm  = pDataArrayNorm->GetTuple(0);
    double *pScale = pDataArrayScale->GetTuple(0);
    double *pVals  = pDataArrayVals->GetTuple(0);

    int iNumVals = pDataArrayVals->GetNumberOfComponents();
    
    if (m_vColors.size() < iNumVals) {
        debug1 << "QHG: [avtPieMapper::InsertFilters] ERROR: haven't got enough colors: numcols " << m_vColors.size() << ", numvals " << iNumVals << endl << std::flush;
    }
        
    // send the pertinent arrays to the vtkSinglePieFilter
    m_ppSinglePieFilter[dom]->SetPieData(pPos, pNorm, pScale, iNumVals, pVals);
    
    return m_ppSinglePieFilter[dom]->GetOutputPort();
}


// ****************************************************************************
//  Method: avtPieMapper::SetColors
//
//  Purpose:
//    set the colors to be used to draw glyphs
//
//  Programmer: jodyxha
//  Creation:   Nov 01, 2021
//
//  Modifications:
//
// ****************************************************************************
void
avtPieMapper::SetColors(colvec &vColors) 
{
    m_vColors = vColors;
    m_pGM->setGlyphColors(m_vColors);
}


// ****************************************************************************
//  Method: avtPieMapper::extractComponentNames
//
//  Purpose:
//    etracts the component names from the input tree
//
//  Programmer: jodyxha
//  Creation:   Nov 01, 2021
//
//  Modifications:
//
// ****************************************************************************
void
avtPieMapper::extractComponentNames() 
{

    avtDataTree_p pTree = GetInputDataTree();

    int iNumArrays = 0;
    vtkDataSet **ppArrays = pTree->GetAllLeaves(iNumArrays);
    debug1 << "QHG: [avtPieMapper::extractComponentNames] got " << iNumArrays << " " <<((iNumArrays == 1)?"Leaf":" Leaves") << endl << std::flush;
    if (iNumArrays > 0) {
        vtkDataSet *curDS = ppArrays[0];

        vtkFieldData *pFieldData = curDS->GetFieldData();
        vtkAbstractArray *pADataArray = pFieldData->GetAbstractArray("names");
        if (pADataArray != NULL) {
            if (pADataArray->GetDataType() == VTK_STRING) {
                vtkStringArray *pSA = vtkStringArray::SafeDownCast(pADataArray);
                debug1 << "QHG: [avtPieMapper::extractComponentNames] as string array: " << pSA << endl << std::flush;
                int iNV =  pSA->GetNumberOfValues();
                debug1 << "QHG: [avtPieMapper::extractComponentNames] number of values incoming string array: "<< iNV << endl  << std::flush;
                m_vComponentNames.clear();
                for (int i  = 0; i < iNV; i++) {
                    m_vComponentNames.push_back(pSA->GetValue(i));
                }
                debug1 << "QHG: [avtPieMapper::extractComponentNames] m_vComponentNames.size()=="<< m_vComponentNames.size() << endl  << std::flush;

            } else {
                // the array "names" should be  of typ VTK_STRING
            }
        } else {
            // no array wuth name "names"
        }

        vtkAbstractArray *pADataArray2 = pFieldData->GetAbstractArray("family");
        if (pADataArray2 != NULL) {
            if (pADataArray2->GetDataType() == VTK_STRING) {
                vtkStringArray *pSA = vtkStringArray::SafeDownCast(pADataArray2);
                m_sFamily = pSA->GetValue(0);
                debug1 << "QHG: [avtPieMapper::extractComponentNames] (" << this << ") got family name [" << m_sFamily << "]"  << endl << std::flush;
            } else {
                // the array "family" should be  of typ VTK_STRING
                
            }
        }else {
            // no array wuth name "family"
        }
    } else {
        // 0 arrays (should not happen)
    }
  
}
