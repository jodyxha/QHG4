#!/usr/bin/python

from sys import argv,stderr,stdout
import h5py


if len(argv) > 1:
    fout=stdout
    if len(argv) > 2:
        fout = open(argv[2])
    #-- end if
    
    f=h5py.File(argv[1], "r")
    lons=list(f["/Geography/Longitude"])
    lats=list(f["/Geography/Latitude"])

    for i in range(len(lons)):
        fout.write("Cell_%03d\t%+12.6f\t%+12.6f\t200\t10\n"%(i,lons[i],lats[i]))
    #-- end for
    
    if len(argv) > 2:
        fout.close()
    #-- end if

    f.close()
#-- end if
