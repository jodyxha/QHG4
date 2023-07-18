#!/usr/bin/python

from sys import argv, exit
import os

linkfiles = {"core"       : ["PopBase.h"],
             "actions"    : ["Action.h",
                             "Action.cpp"], 
             "populations": ["PopulationFactory.h",
                             "DynPopFactory.h",
                             "DynPopFactory.cpp"]
             }

qhg_base = os.environ.get("QHG4_DIR")

for subdir in linkfiles:
    for f in linkfiles[subdir]:
        fCur = "./"+f
        if os.path.exists(fCur):
            os.remove(fCur)
        #-- end if
        print("linking %s %s"%("%s/%s/%s"%(qhg_base,subdir,f), fCur))
        os.symlink("%s/%s/%s"%(qhg_base, subdir, f), fCur)
    #-- end if
#-- end for
