#include <cstdlib>
#include <cmath>
#include <cstring>
#include "utils.h"
#include "strutils.h"

#include <omp.h>

#include "IcoGridNodes.h"
#include "IcoNode.h"

#include "Climate.h"
#include "Geography.h"
#include "Vegetation.h"

#include "SCellGrid.h"

//-----------------------------------------------------------------------------
//  constructor
//
Vegetation::Vegetation(uint iNumCells, int iNumVegSpecies, Geography *pGeography, Climate *pClimate)
    : m_iNumCells(iNumCells),
      m_iNumVegSpecies(iNumVegSpecies),
      m_fPreviousTime(-1.0),
      m_pGeography(pGeography),
      m_pClimate(pClimate) {

    m_adBaseANPP = new double[m_iNumCells];
    m_adTotalANPP = new double[m_iNumCells];
    memset(m_adBaseANPP, 0, m_iNumCells*sizeof(double));
    memset(m_adTotalANPP, 0, m_iNumCells*sizeof(double));
}



//-----------------------------------------------------------------------------
//  destructor
//
Vegetation::~Vegetation() {
    delete[] m_adBaseANPP;
    delete[] m_adTotalANPP;

}


//-----------------------------------------------------------------------------
//  update
//
int Vegetation::update(float fTime) {
    int iResult = 0;
   
    climateUpdate(fTime);
    m_fPreviousTime = fTime;
    
    // nothing else to do 
    
    return iResult;
}


//-----------------------------------------------------------------------------
//  climateUpdate
//
int Vegetation::climateUpdate(float fTime) {
    int iResult = 0;
    
    // nothing to do
    
    return iResult;
}


