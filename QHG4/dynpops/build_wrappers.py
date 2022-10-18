#!/usr/bin/python
from sys import argv
import glob

subst_pattern_nrm = "@@@xxx@@@"
subst_pattern_cap = "@@@XXX@@@"
temp_h   = "WrapperTemplate.h.tmp"
temp_cpp = "WrapperTemplate.cpp.tmp"

def subst_file(fIn, fOut, new_name):
    for line in fIn:
        fOut.write(line.replace(subst_pattern_nrm, new_name).replace(subst_pattern_cap, new_name.upper()))
    #-- end for
#-- end fort


def process_single_item(sStem):
    fOut = open(sStem+"Wrapper.h", "wt")
    fIn = open(temp_h, "rt")
    subst_file(fIn, fOut, sStem)
    fOut.close()
    fIn.close()

    fOut = open(sStem+"Wrapper.cpp", "wt")
    fIn = open(temp_cpp, "rt")
    subst_file(fIn, fOut, sStem)
    fOut.close()
    fIn.close()
#-- end def

if (len(argv) > 1):
    sPopDir = argv[1]

    candidates = glob.glob(sPopDir + "/*Pop.o")
    items= [x.split("/")[-1].replace(".o","") for x in candidates]
    for x in items:
        process_single_item(x)
    #-- end for
