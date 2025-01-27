/** ***************************************************************************\
*   \file   PrGeoProvider.h
*   \author jody
*   \brief  Header file for class SimpleGeoProvider
*
*   PrGeoProvider is a class whic provides geo infomation by means of a
*   Projector object
*** ***************************************************************************/
#ifndef __PRGEOPROVIDER_H__
#define __PRGEOPROVIDER_H__

#include "qhg_consts.h"
#include "DEM.h"
#include "GeoProvider.h"

class ProjGrid;
class GridProjection;
class Projector;

/** ***************************************************************************\
*   \class  PrGeoProvider
*   \brief  cass SimpleGeoProvider
*
*   PrGeoProvider is provides geo information by means of a Projector object.
*** ***************************************************************************/
class PrGeoProvider : public GeoProvider {
public:
    /** ***************************************************************************\
    *   \fn     PrGeoProvider(int iGridW, 
    *                         int iGridH, 
    *                         double dWidth, 
    *                         double dHeight, 
    *                         double dRadius, 
    *                         Projector *pProj,
    *                         bool bDeleteProjector=true,
    *                         bool bDeleteDem=true);
    *   \brief  constructor
    *
    *   \param  iGridW      grid width  (number of cells in X direction)
    *   \param  iGridH      grid height (number of cells in X direction)
    *   \param  dWidth      real width of grid (i.e. of tangential rectangle)
    *   \param  dHeight     real height of grid (i.e. of tangential rectangle)
    *   \param  dRadius     real radius of sphere
    *   \param  pProj       projector object
    *   \param  bDeleteProjector delete Projector in destructor
    *   \param  bDeleteDEM       delete DEM in destructor
    *
    *   Initializes membervariables and calculate deltas and transformation matrix
    *** ***************************************************************************/
    PrGeoProvider(int iGridW,
                  int iGridH,
                  double dWidth,
                  double dHeight,
                  double dRadius,
                  Projector *pProj,
                  bool bDeleteProjector=true,
                  bool bDeleteDem=true);

    /** ***************************************************************************\
    *   \fn     PrGeoProvider(ProjGrid *pPD, 
    *                         Projector *pProj,
    *                         bool bDeleteProjector=true,
    *                         bool bDeleteDem=true);
    *   \brief  constructor
    *
    *   \param  pProjGrid   a ProjGrid structure containing gridw, gridh, realw, realh, realR
    *   \param  pProj       projector object
    *   \param  bDeleteProjector delete Projector in destructor
    *   \param  bDeleteDEM       delete DEM in destructor
    *
    *   Initializes membervariables and calculate deltas and transformation matrix
    *** ***************************************************************************/
    PrGeoProvider(const ProjGrid *pPG,
                  Projector *pProj,
                  bool bDeleteProjector=true,
                  bool bDeleteDem=true);

    virtual ~PrGeoProvider();

    virtual double getRadius() { return RADIUS_EARTH_KM; };

protected:
    /** ***************************************************************************\
    *   \fn     virtual bool getWorldCoordsImpl(double dGridX, 
    *                                           double dGridY, 
    *                                           double &dLat, 
    *                                           double &dLon, 
    *                                           double &dAlt);
    *   \brief  Get real world coordinates corresponding to grid coordinates
    *
    *   \param  dGridX      grid x coordinate
    *   \param  dGridY      grid y coordinate
    *   \param  dLat        real latitude   (output)
    *   \param  dLong       real longitude  (output)
    *   \param  dAlt        real altitude   (output)
    *
    *   \return true if grid coordinates correspond to a real world position
    *   Same as GetInfo, but corrected with tile offset
    *** ***************************************************************************/
    virtual bool getWorldCoordsImpl(double dGridX, 
                                    double dGridY, 
                                    double &dLon, 
                                    double &dLat, 
                                    double &dAlt);

    /** *************************************************************************** \
    *   \fn     virtual bool getGridCoordsImpl(double dLon, 
    *                                          double dLat, 
    *                                          double &dGridX, 
    *                                          double &dGridY)=0;
    *   \brief  Get real world coordinates corresponding to grid coordinates
    *
    *   \param  dLon        real longitude
    *   \param  dLat        real latitude 
    *   \param  dGridX      grid x coordinate  (output)
    *   \param  dGridY      grid y coordinate  (output)
    *
    *   \return true if world coordinates correspond to a grid position(?)
    *
    *  
    *   This method must be implemented by derived classes
    *** ***************************************************************************/
    virtual bool getGridCoordsImpl(double dLon, 
                                   double dLat, 
                                   double &dGridX, 
                                   double &dGridY);


    /** ***************************************************************************\
    *   \fn     virtual bool getCellAreaImpl(double  dGridXE, 
    *                                        double  dGridYN, 
    *                                        double  dGridXW, 
    *                                        double  dGridYS,
    *                                        double &dArea)=0;
    *   \brief  Calculate area of spherical rectangle corresponding to
    *    grid rectangle given by coords
    *
    *   \param  dGridXE      eastern x coordinate
    *   \param  dGridYN      northern y coordinat
    *   \param  dGridXW      western x coordinat
    *   \param  dGridYS      southern y coordinate
    *   \param  dArea        area
    *
    *   \return true if world coordinates correspond to a grid position(?)
    *
    *  
    *   This method must be implemented by derived classes
    *** ***************************************************************************/
     virtual bool getCellAreaImpl(double  dGridXE, 
                                  double  dGridYN, 
                                  double  dGridXW, 
                                  double  dGridYS,
                                  double &dArea);


    Projector      *m_pProj;
    GridProjection *m_pGP;
    double          m_dDefAlt;

};



#endif
