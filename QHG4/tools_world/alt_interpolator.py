#!/usr/bin/python

from sys import argv, stderr, exit
import numpy as np
from scipy.interpolate import RectBivariateSpline
import netCDF4 as nc
import h5py
from QHGError import QHGError


#-----------------------------------------------------------------------------
#--
#--
class AltitudeInterpolator:
    
    #-------------------------------------------------------------------------
    #-- constructor
    #--
    def __init__(self, altitude_nc):
        self.alt_rbs = None
        #print("Initialising AltitudeInterpolator")
        try:
            ncdata = nc.Dataset(altitude_nc, 'r')
        except:
            print("Couldn't open [" + altitude_nc + "] as NC file")
        else:
            try:
                lon = ncdata.variables['x']
                lat = ncdata.variables['y']
                alt = ncdata.variables['z']
            except:
                print("ERROR: netCDF data not found")
            else:
                lon = np.array(lon)
                lat = np.array(lat)  
                alt = np.array(alt)
                self.alt_rbs = RectBivariateSpline(lat, lon, alt)
 
            #-- end try
            ncdata.close()
        #-- end try
    #-- end def


    #-------------------------------------------------------------------------
    #-- doInterpolation
    #--
    def doInterpolation(self, geogrid_qdf):
        iResult = -1
        if (self.alt_rbs != None):
            print("ok - interpolating")
            try:
                qdf = h5py.File(geogrid_qdf,'r+')
            except:
                 print("Couldn't open [" + geogrid_qdf + "] as QDF file")
            else:
                try:
                    geogroup = qdf["/Geography"]
                except:
                    print("ERROR: Geography group not found in " + argv[2])
                else:
                   if geogroup.__contains__('Altitude'):
                        del geogroup['Altitude']
                   #-- end if
                   qLon = geogroup['Longitude'][:]
                   qLat = geogroup['Latitude'][:]
                   nCells = geogroup.attrs['NumCells']
                   MAP = np.float64(self.alt_rbs.ev(qLat, qLon))

                   geogroup.create_dataset("Altitude", MAP.shape, MAP.dtype, MAP)
                   iResult = 0
                   qdf.flush()
                   qdf.close()
                #-- end try
            #-- end try
        else:
            print("AltitudeInterpolator not properly initialised")
        #-- end if
        return iResult
    #-- end def
    
#-- end class


#-------------------------------------------------------------------------
#-- main
#--
if __name__ == '__main__':
    iResult = -1

    if (len(argv) > 3):
        try:
            aip = AltitudeInterpolator(argv[1])
        except QHGError as e:
            print("Constructor for AlititudeInterpolator failed: %s" % e)
        else:
            iResult = aip.doInterpolation(argv[3])
            if (iResult == 0):
                pass
                #print("successfully applied alt-interpolator")
            else:
                print("alt-interpolator failed")
        #-- end try
            
            
    else:
        print("usage")
        print("  %s <alt-ncfile> <time> <qdf-to-modify>" % argv[0])
    #-- end if
    exit(iResult)
#-- end main
