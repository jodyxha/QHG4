#!/usr/bin/python

from sys import argv, exit
import numpy as np
import h5py
#import attr_tools
import PopAttrs

from QHGError import QHGError

default_pairs = [
    ["AltWeight",            "Multi_weight_alt"],
    ["CondWeightedMoveProb", "CondWeightedMove_prob"],
    ["ConfinedMoveX",        "ConfinedMove_x"],
    ["ConfinedMoveY",        "ConfinedMove_y"],
    ["ConfinedMoveR",        "ConfinedMove_r"],
    ["LinearBirth_B0",       "LinearBirth_b0"],
    ["LinearDeath_D0",       "LinearDeath_d0"],
    ["NavigateDecay",        "Navigate_decay"],
    ["NavigateDist0",        "Navigate_dist0"], 
    ["NavigateMinDens",      "Navigate_min_dens"],
    ["NavigateProb0",        "Navigate_prob0"],
    ["NPPWeight",            "Multi_weight_npp"],
    ["RandMove1DProb",       "RandMove1D_prob"],
    ["RandMoveProb",         "RandMove_prob"],
    ["WeightedMoveProb",     "WeightedMove_prob"],
    ["WeightedMoveRandProb", "WeightedMoveRand_prob"],
]

#-----------------------------------------------------------------------------
#-- rename
#--
def rename(loc_attrs, sFrom, sTo):
    k = loc_attrs[sFrom]
    loc_attrs[sTo] = k
    del loc_attrs[sFrom]
#-- end def


#-----------------------------------------------------------------------------
#-- read_rules_file
#--
def read_rules_file(rule_file):
    pairs=[]
    f = open(rule_file, "r")
    for line in f:
        print("line is [%s]"%  (line))
        l = line.strip()
        if (l != ""):
            a = line.strip().split(":")
            pairs.append(a)
        #-- end if
    #-- end for
    return pairs
#-- end def


#-----------------------------------------------------------------------------
#-- read_rules_string
#--
def read_rules_string(rule_string):
    pairs=[]
    rules = rule_string.split(",")

    for rule in rules:
        print("rule is [%s]"%  (rule))
        a = rule.strip().split(":")
        pairs.append(a)
        #-- end if
    #-- end for
    return pairs
#-- end def


#-----------------------------------------------------------------------------
#-- read_rules_file
#--
def read_rules_file(rule_file):
    pairs=[]
    f = open(rule_file, "r")
    for line in f:
        print("line is [%s]"%  (line))
        l = line.strip()
        if (l != ""):
            a = line.strip().split(":")
            pairs.append(a)
        #-- end if
    #-- end for
    return pairs
#-- end def



if len(argv) > 1:
    iResult = 0
    inext = 1
    pairs = []
    if (argv[1] == '-f') or  (argv[1] == '-r'):
        if (len(argv) > 3):
            
            if (argv[1] == '-f'):
                pairs = read_rules_file(argv[2])
                inext = 3
            elif (argv[1] == '-r'):
                pairs = read_rules_string(argv[2])
                inext=3
            else:
                print("unknown option [%s]\n", argv[1])
                iResult = -1
            #--end if
        else:
            printf("expected argument adter '-f' or '-r'")
        #-- end if
    else:
        print("using default renames\n")
        pairs = default_pairs
        inext = 1
    #-- end if

    if (iResult == 0):
        try:
            print("Opening %s" % (argv[inext]))
            #hf = h5py.File(argv[inext], 'r+')
            #loc_attrs = attr_tools.get_attrs(hf, '')
            pa = PopAttrs(argv[inext])
            loc_attrs = pa.read_all_attrs('')
        except Exception as e:
            print(e)
        else:
            for pair in pairs:

                sFrom = pair[0]
                sTo   = pair[1]
                if loc_attrs.__contains__(sFrom):
                    rename(loc_attrs, sFrom, sTo)
                #-- end if 
            #-- end for
            hf.flush()
            hf.close()
        #-- end try
    #-- end if

else:
    print("usage:")
    print("  %s [ -f <rule_file> | -r <rule_string>] <qdf_file>\n" % (argv[0]))
    print("where")
    print("  rule_string ::=  <rename_def>(,<rename_def>)*")
    print("  rename_def  ::=  <old_attr_name>:<new_attr_name>")
    print("  rule_file:       file consisting of lines containing a '<rename_def>'")
    print("If both '-r' and '-f' are omitted the following default rules are used:")
    for p in default_pairs:
        print("  %s:%s" % (p[0], p[1]))
    #-- end for
                           
#-- end try
