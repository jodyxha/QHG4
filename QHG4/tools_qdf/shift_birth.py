#!/usr/bin/python

from sys import argv
import h5py
import numpy
import shutil
import time
import random
from QHGError import QHGError

NAME_POP = "Populations"
NAME_AGS = "AgentDataSet"

#-----------------------------------------------------------------------------
#-- QDFBirthShifter
#--
class QDFBirthShifter:

    #-----------------------------------------------------------------------------
    #-- constructor
    #--
    def __init__(self, qdf_file, pop_name):
        try:
            self.f = h5py.File(qdf_file, "r+")
        except Exception as e:
            raise QHGError(e)
        #--end try
      
        if NAME_POP in self.f:
            self.popgroup = self.f[NAME_POP]
            if (not pop_name) :
                pop_name = list(self.popgroup.keys())[0]
            #-- end if
            
            if pop_name in self.popgroup:
                self.pop = self.popgroup[pop_name]
                if NAME_AGS in self.pop:
                    self.ags =self.pop[NAME_AGS]
                
                else:
                    raise QHGError("No agent dataset [%s] in pop group"%NAME_AGS)
                #-- end if
                
                self.ags2 = []
            else:
                raise QHGError("No group [%s] found in in pop group"%(pop_name))
            #-- end if
                
        else:
            raise QHGError("Didn't find pop group [%s] in [%s]"% (NAME_POP, qdf_file))
        #-- end if
    #-- end def


    #-----------------------------------------------------------------------------
    #-- shift_births
    #--   we assume the agent record lloks like this:
    #--     uint     m_iLifeState;
    #--     int      m_iCellIndex;
    #--     idtype   m_ulID;
    #--     gridtype m_ulCellID;
    #--     float    m_fBirthTime;
    #--     uchar    m_iGender; 
    #--     float    m_fAge
    #--     float    m_fLastBirth; 
    #--     ...
    #--
    def shift_births(self, diff):
        for i in range(len(self.ags)):
            self.ags2.append(self.ags[i])
            self.ags2[i][4] = self.ags2[i][4] - diff
            self.ags2[i][7] = self.ags2[i][7] - diff
        
        #-- end for
        del  self.pop[NAME_AGS]
        self.pop[NAME_AGS] = numpy.array(self.ags2)
        self.f.flush()
        self.f.close()
    #-- end def

#-- end class


if len(argv) > 2:
    
    filepop  = argv[1]

    t0 = 0
    try:
        diff = int(argv[2])

        parts = filepop.split(':')
        qdf_file = parts[0]
        if (len(parts) > 1):
            pop_name = parts[1]
        else:
            pop_name = ""
        #-- end if


        shutil.copyfile(qdf_file, qdf_file+".bak");
        
        t0 = time.time()
        
        qd = QDFBirthShifter(qdf_file, pop_name)
        qd.shift_births(diff)
    except QHGError as qhge: 
        print("Error: [%s]" % (qhge))
    except Exception as e:
        print("Error: [%s]" % (e))
    #-- end try
    t1 = time.time()
    print("Used %f secs\n" % (t1-t0))
else:
    print("Subtract a number of years from the birth time of each agent of a population")
    print("Usage:")
    print("  %s <qdf-file>[:<pop-name>] <amount>" % argv[0])
    print("where")
    print("  qdf-file   QDF file to modify")
    print("  popname    name of population's group. If omitted, the first population is used")
    print("  amount     number of years to subtract from the birth times")
    print("")
    print("A backup file named <qdfile>.bak will be made")
#-- end if









