// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ****************************************************************************
//  File: GlyphManager.C
// ****************************************************************************

#include <cstdio>
#include <cmath>

#include <sstream>
#include <DebugStream.h>

#include "GlyphManager.h"
#include "GlyphGeometry.h"
#include "PieGeometry.h"
#include "BarGeometry.h"
#include "BoxGeometry.h"

const int   DEF_NUM_SECTORS = 120;
const float DEF_RAD_PIE     = 1.0; 
const float DEF_RAD_BORDER  = 1.1; 
const float DEF_BAR_SCALE_X = 1.0;
const float DEF_BAR_SCALE_Y = 1.0;
const float DEF_BAR_BORDER  = 0.1;
const float DEF_BOX_SCALE_X = 1.0;
const float DEF_BOX_SCALE_Y = 1.0;
const float DEF_BOX_BORDER  = 0.1;

GlyphManager *GlyphManager::_instance = NULL;

// ****************************************************************************
//  Method: GlyphManager::createInstance
//
//  Purpose:
//    get a pointer to the GlyphManager singleton.
//    Create GlyphManager singleton if it doesn't already exist
//
//  Returns;    pointer to the GlyphManager singleton
//
//  Programmer: jodyxha
//  Creation:   Wed Oct 6 11:02:10 PDT 2021
//
// ****************************************************************************

GlyphManager *
GlyphManager::createInstance() 
{
  
    if (_instance == NULL) {
        _instance = new GlyphManager();
        int iResult = _instance->init();
        if (iResult != 0) {
            delete _instance;
            _instance = NULL;
        }
    }

    return _instance;
}



// ****************************************************************************
//  Method: GlyphManager constructor
//
//  Programmer: jodyxha
//  Creation:   Wed Oct 6 11:02:10 PDT 2021
//
// ****************************************************************************

GlyphManager::GlyphManager() 
    :  m_iNumSectors(DEF_NUM_SECTORS),
       m_fRadiusPie(DEF_RAD_PIE),
       m_fRadiusBorder(DEF_RAD_BORDER),
       m_fBarScaleX(DEF_BAR_SCALE_X),
       m_fBarScaleY(DEF_BAR_SCALE_Y),
       m_fBarBorder(DEF_BAR_BORDER),
       m_fBoxScaleX(DEF_BOX_SCALE_X),
       m_fBoxScaleY(DEF_BOX_SCALE_Y),
       m_fBoxBorder(DEF_BOX_BORDER)
{

}


// ****************************************************************************
//  Method: PieGometry destructor
//
//  Programmer: jodyxha
//  Creation:   Wed Oct 6 11:02:10 PDT 2021
//
// ****************************************************************************

GlyphManager::~GlyphManager()
{

}


// ****************************************************************************
//  Method: GlyphManager::init
//
// Purpose:
//   perform initialisation after the constructor has been called
//
//  Returns:     error code (0: success, negaive number: error)
//
//  Programmer: jodyxha
//  Creation:   Wed Oct 6 11:02:10 PDT 2021
//
// ****************************************************************************

int 
GlyphManager::init()
{
    return 0;
}



// ****************************************************************************
//  Method: GlyphManager::setPieAtts
//
// Purpose:
//   store the attributes for the pies
//
//  Arguments:
//      iNumSectors    number of sectors to use (more sectors -> rounder pie)
//      fRadPie        radius of the value slices
//      fPieBorder      border thickness
//
//
//  Programmer: jodyxha
//  Creation:   Wed Oct 6 11:02:10 PDT 2021
//
// ****************************************************************************

void 
GlyphManager::setPieAtts(int iNumSectors, float fRadPie, float fPieBorder) 
{
    m_iNumSectors   = iNumSectors;
    m_fRadiusPie    = fRadPie;
    m_fRadiusBorder = fPieBorder + fRadPie;
}


// ****************************************************************************
//  Method: GlyphManager::setBarAtts
//
// Purpose:
//   store the attributes for the pies
//
//  Arguments:
//      fScaleX        width of bar diagrams
//      fScaleY        height of bar diagrams
//      fBarBorder     border thickness
//
//
//  Programmer: jodyxha
//  Creation:   Wed Oct 6 11:02:10 PDT 2021
//
// ****************************************************************************

void 
GlyphManager::setBarAtts(float fScaleX, float fScaleY, float fBarBorder) {
    m_fBarScaleX = fScaleX;
    m_fBarScaleY = fScaleY;
    m_fBarBorder = fBarBorder;
}


// ****************************************************************************
//  Method: GlyphManager::setBoxAtts
//
// Purpose:
//   store the attributes for the pies
//
//  Arguments:
//      fScaleX        width of box diagrams
//      fScaleY        height of box diagrams
//      fBoxBorder     border thickness
//
//
//  Programmer: jodyxha
//  Creation:   Wed Oct 6 11:02:10 PDT 2021
//
// ****************************************************************************

void 
GlyphManager::setBoxAtts(float fScaleX, float fScaleY, float fBoxBorder) {
    m_fBoxScaleX = fScaleX;
    m_fBoxScaleY = fScaleY;
    m_fBoxBorder = fBoxBorder;
}


// ****************************************************************************
//  Method: GlyphManager::setGlyphColors
//
// Purpose:
//   set new colors for the values
//
//  Arguments:
//      vColors        a vector containing the color for every value
//
//
//  Programmer: jodyxha
//  Creation:   Wed Oct 6 11:02:10 PDT 2021
//
// ****************************************************************************
void 
GlyphManager::setGlyphColors(colvec &vColors) 
{
    m_vGlyphColors.clear();
    m_vGlyphColors  = vColors; 
    debug1 << "QHG:[GlyphManager::setGlyphColors]  got colors:" <<endl <<std::flush;
    for (uint i = 0; i < m_vGlyphColors.size(); i++) {
        debug1 << "QHG:[GlyphManager::setGlyphColors]    ("<<m_vGlyphColors[i][0]<<", "<<m_vGlyphColors[i][1]<<", "<<m_vGlyphColors[i][2]<<", "<<m_vGlyphColors[i][3]<<")" <<endl <<std::flush;
    }
}


// ****************************************************************************
//  Method: GlyphManager::addGlyph
//
// Purpose:
//   create a new glyph based on  the provided data
//
//  Arguments:
//      iType         glyph type
//      pPoint        glyph position
//      pNormal       glyph orientation
//      fScale        scaling for all glyphs in plot
//      iNumValues    number of values 
//      pValues       the values
//
//
//  Programmer: jodyxha
//  Creation:   Wed Oct 6 11:02:10 PDT 2021
//
// ****************************************************************************
vtkPolyData *
GlyphManager::createGlyph(uint iType, const double *pPoint, const double *pNormal, double fScale, int iNumValues, const double *pValues) 
{
    vtkPolyData *pPD = NULL;

    bool bShowDetails = false;
    GlyphGeometry *pGG = NULL;
    if (m_vGlyphColors.size() > 0) {
        if (iType == 0) {
            if (bShowDetails) {debug1 << "QHG: [GlyphManager::addGlyph] (" << this <<") creating a pie with numsectors=" << m_iNumSectors << ", radpie=" << m_fRadiusPie << ", piebord="<< m_fRadiusBorder<<endl <<std::flush;}
            pGG = new PieGeometry(m_iNumSectors, m_fRadiusPie, m_fRadiusBorder, m_vGlyphColors);
            //pGG = new PieGeometry(120, 1.0, 1.1);
        } else if (iType == 1) {
            if (bShowDetails) {debug1 << "QHG: [GlyphManager::addGlyph] (" << this <<")  creating a bar with scalex=" << m_fBarScaleX << ", scaley=" << m_fBarScaleY << ", barbord=" << m_fBarBorder << endl <<std::flush;}
            pGG = new BarGeometry(m_fBarScaleX, m_fBarScaleY, m_fBarBorder, m_vGlyphColors);
        } else if (iType == 2) {
            if (bShowDetails) {debug1 << "QHG: [GlyphManager::addGlyph] (" << this <<")  creating a box with scalex=" << m_fBoxScaleX << ", scaley=" << m_fBoxScaleY << ", boxbord=" << m_fBoxBorder << endl <<std::flush;}
            pGG = new BoxGeometry(m_fBoxScaleX, m_fBoxScaleY, m_fBoxBorder, m_vGlyphColors);
        } else {
            printf("QHG: [GlyphManager::addGlyph] (%p) unknown glyphtype (only know 0(Pie), 1 (Bar), 2 (Box))\n", this);
        }

        if (pGG != NULL) {
            if (bShowDetails) {debug1 << "QHG: [GlyphManager::addGlyph] geometry created ok" <<endl <<std::flush;}
            pGG->createPolyData(pPoint, pNormal, 1.0, iNumValues, pValues);
            if (pGG != NULL) {
                pPD = pGG->getDataSet();
                if (bShowDetails) {debug1 << "QHG: [GlyphManager::addGlyph] added glyph data set" <<endl <<std::flush;}
        
                //        debug1 << "QHG: [GlyphManager::addGlyph] (" << this <<") have " <<     m_mGlyphGeoms.size() << " geometries now" << endl << std::flush;
                //delete pGG;
            } else {
                debug1 << "QHG: [GlyphManager::addGlyph] error creating glyph data set" <<endl <<std::flush;
            }
        } else {
            debug1 << "QHG: [GlyphManager::addGlyph] error creating geometry" <<endl <<std::flush;
        }
    } else {
        debug1 << "QHG: [GlyphManager::addGlyph] got empty colors vector" <<endl <<std::flush;
    }
    return pPD;
}

                           
// ****************************************************************************
//  Method: GlyphManager::createTransform
//
// Purpose:
//   calculate the transform for specified  translation, rotation and scale
//
//  Arguments:
//      pPoint        glyph position
//      pNormal       glyph orientation
//      pScale        scaling for all glyphs in plot
//      bUpright      if true, the plots' local y axes are aligned as well 
//                    as possible with the global z directoiniNumValues    number of values 
//
//  Returns:  vtkTransform 
//
//  Programmer: jodyxha
//  Creation:   Wed Oct 6 11:02:10 PDT 2021
//
// ****************************************************************************
vtkTransform *
GlyphManager::calcTransform(const double *pPoint, const double *pNormal, const double *pScale, bool bUpright) 
{
    // now the rotation: we want z=(0,0,1) to rotate into pNormal;
    // rotation angle: angle between z and pNormal

    double vZ[3]{0, 0, 1};
    double vN[3]{pNormal[0], pNormal[1], pNormal[2]};

    vtkMath::Normalize(vN);
    double angle = acosf(vtkMath::Dot(vZ, vN))*180/M_PI;
    
    // rotation axis:vertical to plane given by z and pNormal
    double axis[3];
    vtkMath::Cross(vZ, vN, axis);
    
    vtkTransform *pTransform = vtkTransform::New();
    pTransform->PostMultiply();
    pTransform->RotateWXYZ(angle, axis);

    if (bUpright) {
        // this will align the glyph's local y-axis to the global z-axis as well as possible
        double vY[3]{0, 1, 0};
        double vYp[3];
        pTransform->TransformPoint(vY, vYp);
        vtkMath::Normalize(vYp);
        double vZp[3];
        double vNs[3];
        memcpy(vNs, vN, 3*sizeof(double)); //vtkMath::Assign(vN, vNs);

        double dDot = vtkMath::Dot(vZ, vNs);
        vtkMath::MultiplyScalar(vNs, dDot);
        vtkMath::Subtract(vZ, vNs, vZp);
        if (vtkMath::Norm(vZp) >1E-6) {
            vtkMath::Normalize(vZp);
            double angle2 = acosf(vtkMath::Dot(vYp, vZp))*180/M_PI;
            double cr[3];
            vtkMath::Cross(vYp, vZp, cr);
            pTransform->RotateWXYZ(angle2, cr);
        }
    } 


    pTransform->Scale(pScale);
    pTransform->Translate(pPoint);
    return pTransform;
}
