#!/usr/bin/env python

import netCDF4 as nc
import numpy   as np
import time    as ttt

def walktree(top):
    values = top.groups.values()
    yield values
    for value in top.groups.values():
        for children in walktree(value):
            yield children
        #-- end for
    #-- end for
#-- end def

sources_old = {"784NPP": ('http://climatedata.ibs.re.kr:9090/dods/public-data/loveclim-784k/loveclim-784k-netpp', 'time', 'lat', 'lon', 'netpp', -9.990000e+08),
               "407NPP": ('core_data/npp_timmermann.nc', 'TAXI', 'YAXLEVITR', 'XYAXLEVITR', 'NPPHRS', -1.00000000e+34)}

sources = {"784NPP": ('new1.nc', 'mtime', 'lat', 'lon', 'netpp', -9.990000e+08),
           "407NPP": ('core_data/timmermannR.nc', 'time', 'YAXLEVITR', 'XYAXLEVITR', 'NPPHRS', -1.00000000e+34)}


#url = 'http://climatedata.ibs.re.kr:9090/dods/public-data/loveclim-784k/loveclim-784k-netpp'
#url = 'new1'
key='407NPP'
url = sources[key][0]
dataset = nc.Dataset(url)
T_NAME   = sources[key][1]
LAT_NAME = sources[key][2]
LON_NAME = sources[key][3]
NPP_NAME = sources[key][4]
FILL_VAL = sources[key][5]

print("url: "+url);
print("T:   "+T_NAME)
print("Y:   "+LAT_NAME)
print("X:   "+LON_NAME)
print("N:   "+NPP_NAME)
print("----- Original -----")
print(dataset)
for children in walktree(dataset):
    for child in children:
        print(child)
print("--------------------")


for k in dataset.variables.items():
    print(k)
    vvv = dataset.variables[k[0]]
   
    if len(vvv.dimensions) == 1:
        print(vvv[:])
    elif  len(vvv.dimensions) == 3:
        vvv.set_auto_mask(False)
    #--ensd if


# lookup a variable
time0 = dataset.variables[T_NAME]
if len(time0.dimensions) > 1:
    rtime = np.array(time0[...,0,0])
else:
    rtime = np.array(time0)
#-- end if
    
lat0  = dataset.variables[LAT_NAME]
npp0  = dataset.variables[NPP_NAME]
#npp0.set_auto_mask(False)
print(npp0[0][16])
print(npp0.dimensions)
# this does not work for 784NPP but it does for 407NPP
npp1=np.array(npp0)
npp1[npp1 == FILL_VAL] = 0

t=0
print("--- contents "+NPP_NAME+ " at "+str(rtime[t]))
for x in range(len(lat0)):
    print("%f: "%lat0[x])
    print(npp1[t][x][:])
    print("")

"""
if True:
    time0 = dataset.variables['T_NAME']
    lat0  = dataset.variables['lat']
    lon0  = dataset.variables['lon']
    mtime0 = dataset.variables['mtime']
    rtime = np.array(mtime0[...,0,0])
    time0.set_auto_mask(False)
    lat0.set_auto_mask(False)
    lon0.set_auto_mask(False)
    print(npp0.dimensions)
    
    npp1=np.array(npp0)
    
    npp1[npp1 == -9.990000e+08] = 0 #np.NaN
    print(lat0)
    print(lon0)
    print(npp0)
    print(time0)

    print("rtime is:")
    print(rtime)
     
    print("npp:  %d" % len(npp0))
    print("time: %d" % len(time0))
    print("lon:  %d" % len(lon0))
    print("lat:  %d" % len(lat0))


    tt=123
    print("npp for t=%f:"%(time0[tt],))
    print(npp1[tt][14])
    print(npp1[tt][15])
    print(npp1[tt][16])
    print(npp1[tt][17])
    print(npp1[tt][18])
    
    f=npp0[tt]
    print("f is "+str(f))
#-- end if
"""    
