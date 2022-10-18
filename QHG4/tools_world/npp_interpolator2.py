#!/usr/bin/python

from sys import  argv, stderr, exit
import h5py 
import numpy as np
import netCDF4 as nc

import grid_utils

# for timmermann.nc
LON_NAME  = 'XAXLEVITR'
LAT_NAME  = 'YAXLEVITR'
TIME_NAME = 'TAXI'
NPP_NAME  = 'NPPHRS'
# for NPPDO0.nc
#LON_NAME  = 'longitude'
#LAT_NAME  = 'latitude'
#TIME_NAME = 'time'
#NPP_NAME  = 'NETPP'

#-----------------------------------------------------------------------------
#--
#--
class QHGError(Exception):
    def __init__(self, message):
        Exception.__init__(self, message)
    #-- end def
#-- end class

#-----------------------------------------------------------------------------
#--
#--
class NPPInterpolator:
    
    #-------------------------------------------------------------------------
    #-- constructor
    #--
    def __init__(self, npp_file, lon_name, lat_name, time_name, npp_name):
        self.npp_file = npp_file
        try:
            self.ncdata = nc.Dataset(npp_file, 'r')
            self.longs  = np.array(self.ncdata.variables[lon_name])
            self.lats   = np.array(self.ncdata.variables[lat_name])

            self.times  = np.array(self.ncdata.variables[time_name])
            
            #self.ncdata.close()
        except Exception as e:
            raise QHGError("couldn't open %s and extract data. Exception: %s" % (npp_file, e))
        #-- end try
    #--end def


    #-------------------------------------------------------------------------
    #-- destructor
    #--     cose the handle to the ncdata (opened in constructor)
    #--
    def __del__(self):
        self.ncdata.close()
    #-- end def
    
    #-------------------------------------------------------------------------
    #-- findTimeIndex
    #--     time values in timmermann's data go from -407 to -1,
    #--     and we use ky b.p., therefore we have to 'invert' the time 
    #--
    def findTimeIndex(self, t0):
        index = -1
        d     = -1

        t0    = max(self.times) - t0 
        for i in range(self.times.size):
            d1 = abs(t0 - self.times[i])
            if (d < 0) or (d1 < d):
                d = d1
                index = i
            #-- end if
        #-- end for
        return index
    #-- end def

    
    #-----------------------------------------------------------------------------
    #-- doInterpolation
    #--   do bivariate interpolations for npp at given time
    #--   onto the grid defined by qLon and qLat
    #--
    def doInterpolation(self, target_time, qLon, qLat):
        index = self.findTimeIndex(target_time)
        if (index >= 0):
            print("[npp]Index for %f : %d" % (target_time, index))
            print("[npp]qLat ("+str(qLat.shape)+") : %f %f  %f %f %f" %(qLat[0],qLat[1],qLat[2],qLat[3],qLat[4]))
            npp_cur  = np.array(self.ncdata.variables[NPP_NAME][index])
            npp_temp = np.where(npp_cur < 0, 0, npp_cur)
            #print("temp: %d values, min: %f, max %f" % (npp_temp.size, min(npp_temp), max(npp_temp)))
            print("[npp]temp: %d values, shape:%s, first %s, min %f, max %f" % (npp_temp.size, str(npp_temp.shape), str(npp_temp[0][0]), npp_temp.min(), npp_temp.max()))
            npp_new =  grid_utils.doGridInterpolation(qLon, qLat, self.longs, self.lats, npp_temp, np.float64, False)
            #print("int: %d values, shape:%s, first %s, min %f, max %f" % (npp_new.size, str(npp_new.shape), str(npp_new[0][0]), npp_new.min(), npp_new.max()))
            print("[npp]int: %d values, shape:%s, min %f, max %f" % (npp_new.size, str(npp_new.shape), npp_new.min(), npp_new.max()))
        #-- end if
       
        return npp_new
    #-- end def

#-- end classq


#-------------------------------------------------------------------------
#-- main
#--
if __name__ == '__main__':
    iResult = -1

    if (len(argv) > 7):
        try:
            t = float(argv[6])
        except:
            print("time [%s] must be a float" % argv[6])
        else:
            try:

                #ni = NPPInterpolator(argv[1])
                ni = NPPInterpolator(argv[1], argv[2], argv[3], argv[4], argv[5])
            except QHGError as e:
                print("Constructor failed: %s" % e)
            except Exception as e:
                print("Constructor failed: %s" % e)
            else:
                print("ataring: time=%f, out=%s"%(t, argv[7]))
                qLon, qLat = grid_utils.extractQDFCoordinates(argv[7])
                npp1 = ni.doInterpolation(t, qLon, qLat)
                npp2 = np.where(npp1 < 0, 0, npp1)
                print("success: %d values (%s), min: %f, max %f" % (npp2.size, str(npp2.shape), npp2.min(), npp2.max()))
                grid_utils.replaceQDFFileArray(argv[7], 'Vegetation', 'NPP', npp2)
                grid_utils.replaceQDFFileArray(argv[7], 'Vegetation', 'BaseNPP', npp2)
                iResult = 0
                
            #-- end try
            
            
    else:
        print("usage")
        print("  %s <npp-ncfile> <lon_name> <lat_name> <time_name> <npp_name> <time> <qdf-to-modify>" % argv[0])
    #-- end if
    exit(iResult)
#-- end main
