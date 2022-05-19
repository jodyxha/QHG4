#!/usr/bin/python 

from sys import argv, exit
from math import *
import numpy as np
import h5py

#-- lon/lat rectangle encompassing oceania
REG_OCEANIA_LONMIN = 115.0;
REG_OCEANIA_LONMAX = 150.0;
REG_OCEANIA_LATMIN = -12.0;
REG_OCEANIA_LATMAX =   1.0;
NPP_MIN = 0.08

#---------------------------------------------------
#-- calcMiami
#--   calculate npp value for given rain and
#--   temperature using the Miami model
#--
def calcMiami(temp, rain, i):
    nppT = 3000.0 / (1 + exp(1.315 - 0.119*temp[i]))
    nppP = 3000.0 * (1 - exp(-0.000664*rain[i]))

    return 0.000475*min(nppT,nppP)
#-- end def
    


#---------------------------------------------------
#-- flatten
#--
def flatten(qdf_file, groups):
    try:
        h_qdf = h5py.File(qdf_file,'r+')
        try:
            geogroup = h_qdf["/Geography"]
        except:
            print("ERROR: Geography group not found in " + qdf_file)
        else:
            if geogroup.__contains__('Altitude'):
                res = 0
                for g in groups:
                    bGlobalUse = ('!' in g[0])
                    sp = g[0].replace('!','')
                    res = flatten_group(h_qdf, sp, g[1], bGlobalUse)
                    if (res < 0):
                        break
                    #-- end if
                #-- end for
                
                if (res == 0):
                    print("flush it")
                    h_qdf.flush()
                    h_qdf.close()
                else:
                    print("failed")
                #-- end if
            else:
                print("No Geography group found in [%s]"%qdf_file)
            #-- end if
        #-- end try
    except Exception as e:
        print(e)   

        print("Couldn't open [" + qdf_file + "] as QDF file")
    #-- end try
#-- end def
        
        
#---------------------------------------------------
#-- flatten_group
#--
def flatten_group(h_qdf, specs, def_val, bGlobalUse):

    iResult = 0
    # we already know there's an altitude array
    geogroup = h_qdf["/Geography"]
    alt = np.array(geogroup['Altitude'])
                   

    if (iResult == 0) and ('A' in specs):
        if bGlobalUse:
            print("setting global altitudes to %f" % def_val)
            alt = len(alt)*[def_val]
        else:
            print("setting land altitudes to %f" % def_val)
            alt[np.where (alt > 0)]  = def_val
            alt[np.where (alt <= 0)] = -1
        #-- end if
        del geogroup['Altitude']
        geogroup.create_dataset("Altitude", alt.shape, alt.dtype, alt)
    #-- end if

    if (iResult == 0) and ('W' in specs):
        if geogroup.__contains__('Water'):
            h2o =  np.array(geogroup['Water'])

            if bGlobalUse:
                h2o = len(h2o)*[def_val]
            else:
                print("setting Water values to %f" % def_val)
                h2o[np.where (alt > 0)]  = def_val
                h2o[np.where (alt <= 0)] = 0
            #-- end if
            del geogroup['Water']
            geogroup.create_dataset("Water", h2o.shape, h2o.dtype, h2o)
        else:
            printf("specified 'W' but no Water array found")
            iResult = -1
        #-- end if
    #-- end if

    if (iResult == 0) and ('C' in specs):
        if geogroup.__contains__('Coastal'):
            print("setting Coastal values to %f" % def_val)
            cst =  np.array(geogroup['Coastal'])

            if bGlobalUse:
                cst = len(cst)*[def_val]
            else:
                cst[np.where (alt > 0)]  = def_val
                cst[np.where (alt <= 0)] = 0
                #cst = 0 * cst
            #-- end if
            del geogroup['Coastal']
            geogroup.create_dataset("Coastal", cst.shape, cst.dtype, cst)
        else:
            printf("specified 'C' but no Coastal array found")
            iResult = -1
        #-- end if
    #-- end if

    if (iResult == 0) and ('I' in specs):
        if geogroup.__contains__('IceCover'):
            print("setting IceCover values to %f" % def_val)
            ice = np.array(geogroup['IceCover'])
            ice = def_val*ice
            del geogroup['IceCover']
            geogroup.create_dataset("IceCover", ice.shape, ice.dtype, ice)
        else:
            printf("specified 'I' but no IceCover array found")
            iResult = -1
        #-- end if
    #-- end if
            

    if (iResult == 0) and ('N' in specs):
        try:
            veggroup = h_qdf["/Vegetation"]
        except:
            print("ERROR: vegetation group not found in file")
            iResult = -1
        else:
            if veggroup.__contains__('BaseNPP'):
                bnpp = np.array(veggroup['BaseNPP'])
                
                if veggroup.__contains__('NPP'):
                    tnpp = np.array(veggroup['NPP'])
                    
                    print("setting bnpp and tnpp to %f" % (def_val))

                    if bGlobalUse:
                        bnpp = len(bnpp)*[def_val]
                        tnpp = len(tnpp)*[def_val]
                    else:
                        bnpp[np.where (alt > 0)]  = def_val
                        bnpp[np.where (alt <= 0)] = -1
                        tnpp[np.where (alt > 0)]  = def_val
                        tnpp[np.where (alt <= 0)] = -1
                    #-- end if
                    
                    del veggroup['BaseNPP']
                    veggroup.create_dataset("BaseNPP", bnpp.shape, bnpp.dtype, bnpp)

                    del veggroup['NPP']
                    veggroup.create_dataset("NPP", tnpp.shape, tnpp.dtype, tnpp)

                    iResult = 0
                else:
                    printf("specified 'N' but no NPP array found")
                    iResult = -1
                #-- end if
            else:
                printf("specified 'N' but no BaseNPP array found")
                iResult = -1
            #-- end if
        #-- end try
    #-- end if

    
    return iResult
#-- end def


#---------------------------------------------------
#-- usage
#--
def usage(appname):
    print("%s - flatten a qdf file" % (appname))
    print("Usage:")
    print("  %s  <qdf_file> (<what>*:<what_val>)*" % (appname))
    print("where")
    print("  what        'A' | 'W' | 'C' | 'I' | 'N' | '!'")
    print("              'A':Altitude")     
    print("              'W':Water")     
    print("              'C':Coastal")     
    print("              'I':Icecover")     
    print("              'N':npp")     
    print("              '!':apply altitude to *all* cells, not only land cells")     
    print("  what_val    value to assign to arrays specified in <what>*")
    print("Example:")
    print("  %s file1.qdf A:130 WCI:0 N:0.8" % (appname))
    print("Set land altitudes to 130m, set water values and coastal values to 0; set ice cover to 0; set NPP to 0.8"); 
#-- end def



#---------------------------------------------------
#-- main
#--  expect
#--     <qdf_file> ["alt" <alt_val>]] ["npp" <npp_val>] ["ice" <ice_val>]
#--
if (len(argv) > 1):
    qdf_file = argv[1]
    bDoAlt      = False
    bDoNPP      = False
    bDoIce      = False
    bDoWater    = False
    bDoCoastal  = False
    alt_val     = 0
    npp_val     = 0
    ice_val     = 0
    water_val   = 0
    coastal_val = 0

    groups=[]

    try:
        for i in range(2, len(argv)):
            cur_com = []
            v_arg = argv[i].split(':') 
            if(len(v_arg) == 2):
                cur_val = float(v_arg[1])
                for x in v_arg[0]:
                    if not (x in "AWCIN!"):
                        raise Exception("bad specifier [%s]"%x)
                    #-- end if
                #-- end for
                # if we are here, all characters are ok
                groups.append([v_arg[0], cur_val])
            else:
                raise Exception("Bad argument [%s]"%(argv[i]))
            

            #-- end if
            
        #-- end while
    except Exception as e:
        print(e)
    else:
        flatten(qdf_file, groups)


    #-- end try
else:
    usage(argv[0])
#-- end main
