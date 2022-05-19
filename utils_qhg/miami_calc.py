#!/usr/bin/python

from sys import argv
from math import exp

if (len(argv) >2):
    temp = float(argv[1])
    rain = float(argv[2])

    nppT = 3000.0 / (1 + exp(1.315 - 0.119*temp))
    nppP = 3000.0 * (1 - exp(-0.000664*rain))
    print("t %f gDM, %f kgC" % (nppT, 0.000475*nppT))
    print("r %f gDM, %f kgC" % (nppP, 0.000475*nppP))

    print("%f gDM, %f kgC" % (min(nppT, nppP), 0.000475*min(nppT,nppP)))
else:
    print("%s <avg_temp_celsius> <ann_rain_mm>" % argv[0])
#-- end if
