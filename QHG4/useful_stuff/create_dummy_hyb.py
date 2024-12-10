#!/usr/bin/python

from sys import argv, stdout
import numpy as np

def looper(dims, k, iCur):
    #print("looper(%s,%d,%d)"%(dims, k, iCur))
    if k == len(dims):
        stdout.write("%d "%iCur)
        iCur += 1
    else:
        for d in range(dims[k]):
            iCur = looper(dims, k+1, iCur)
            if k < len(dims)-1:
                print("")
            #-- end if
        #-- end for
    #-- end if
    return iCur
#-- end def 

if len(argv) > 1:
    sdims = argv[1].split(":")
    dims  = [int(x) for x in sdims]
    tot=np.prod(dims)
    print("dimensions: %s"%" ".join(["%s"%x for x in dims]))
    #print("tot: %f"%tot)

    looper(dims, 0, 0)
    
#-- end if
