#!/usr/bin/python
from sys import argv, exit, stderr
import h5py
import numpy as np
import re


#-----------------------------------------------------------------------------
# extractSimNameStep
#   assume file has form
#   <some>/<path>/<simname>/ooa_pop-sapiens_SG_<step>.qdf
#   exclude_list and return them as ';'-separated string
#  
def extractSimNameStep(filename):
    res = None
    pat=".*/([-_+a-zA-Z0-9\.]*)/ooa_pop-sapiens_SG_([0-9]*).qdf"
    m=re.match(pat, filename)
    if (not m is None) and (m.lastindex > 1):
        res = [m[1], m[2]]
    #-- end if
    return res
#-- end def


exclude=["NPPCap_alt_pref_poly_neander", "NPPCap_alt_pref_poly_sapiens", "PrivParamMix_mode"]
#exclude=["NPPCap_alt_pref_poly_neander", "NPPCap_alt_pref_poly_sapiens"]

iResult = -1
if len(argv) > 1:
    atta = []
    vava = []
    separator=';'
    stderr.write("[agentprops_overview] opening [%s]\n"%argv[1])
    h=h5py.File(argv[1], 'r')
    k = extractSimNameStep(argv[1])
    if (not k is None):
        stderr.write("[agentprops_overview] have step %s\n"%(k))
        atta = ['simname', 'step']
        vava.extend(k)
        d = h['Populations/sapiens']
        #print("%s"%list(d.attrs))
        for g in d.keys():
            if isinstance(d[g], h5py._hl.group.Group):
                x = list(d[g].attrs)
                atta.extend(x)
                L = d[g]
                for x in L.attrs:
                    v = L.attrs[x]
                    #stderr.write("have [%s]\n"%str(v))
                    if len(v) == 1:
                        vava.append(v[0])
                    else:
                        vava.append(v[0])
                    #-- end if
                #-- end for
            #-- end if
        #-- end for
    
        indexes = [atta.index(e) for e in exclude]
        indexes.sort(reverse=True)
          
        if (len(argv) > 2) and (argv[2] == '-h'):
            for i in indexes:
                del atta[i]
            #-- end for
            print("%s"%separator.join(atta))
        #-- end if
        
        for i in indexes:
            del vava[i]
        #-- end for
        
        if (len(argv) > 3) and (argv[3] == '-u'):
            vavae=['-']*len(vava)
            vavae[0] = vava[0]
            print("%s"%separator.join([str(x) for x in vavae]))
        else:
            print("%s"%separator.join([str(x) for x in vava]))
        #-- end if
        h.close()
        iResult = 0
        stderr.write("[agentprops_overview] +++ success for [%s]\n"%argv[1])
        #print("now with attr_tools")
        #header = build_attr_header(argv[1], exclude)
        #data   = build_attr_data(argv[1], exclude)

        #print("%s\n"%header);
        #print("%s\n"%data);
    else:
        stderr.write("[agentprops_overview] Couldn't extract simname/step from [%s]\n"%argv[1])
    #-- end if
else:
    print("usage:")
    print("  %s <qdf-file> [-h] [-u]"%argv[0])
    print("where")
    print("  qdf-file   qdf file to extract attributes from")
    print("             should have name like '<some>/<path>/<simname>/ooa_pop-sapiens_SG_<step>.qdf'")
    print("  -h         out header before attribute line")
    print("  -u         print a line of '-' entries")          
#-- end if
exit(iResult)
