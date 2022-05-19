#!/usr/bin/python

from sys import argv
import h5py
import math


class rounder:

    def __init__(self, prec):
        self.prec = pow(10, prec)
    #-- end def

    def fround(self, a):
        f = math.log(a)/math.log(10)
        r = -int(f)+1
        p = pow(10,r)
        u = int(p*a*self.prec)/(p*self.prec)
        return u
    #-- end def
#-- end class

# print out area data of a QDF file
# including rations biggest/smallest and 2ndbiggest/2ndsmallest
if len(argv) > 2:
    h = h5py.File(argv[1], 'r')
    rd = rounder(int(argv[2]))
    a =  sorted(set([rd.fround(x) for x in h['Geography/Area']]))
    
    # we need at least three items
    
    if (len(a) == 1):
        a.extend(a)
        a.extend(a) 

    elif (len(a) == 2):
        # interleave a with itself -> a0,a1,a0,a1
        a =  [x for t in zip(a, a) for x in t]

    #-- end if
    print("%s;%.3e;%.3e;%.3e;%.3e;%.4f;%.4f"%(argv[1], a[0], a[-1], a[1], a[-2], a[-1]/a[0], a[-2]/a[1]))
